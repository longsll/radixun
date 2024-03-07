#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

namespace radixun {

static radixun::Logger::ptr g_logger = RADIXUN_LOG_NAME("system");

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event) {
    switch(event) {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            RADIXUN_ASSERT2(false, "getContext");
    }
}

void IOManager::FdContext::resetContext(EventContext& ctx){
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event){
    RADIXUN_ASSERT(events& event);
    events = (Event)(events & ~event);
    EventContext& ctx = getContext(event);
    if(ctx.cb){
        ctx.scheduler->schedule(&ctx.cb);
    }else{
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return ;
}

IOManager::IOManager(size_t threads , bool use_caller , const std::string& name)
    :Scheduler(threads , use_caller , name){
     // 创建epoll实例
    m_epfd = epoll_create(5000);
    RADIXUN_ASSERT(m_epfd > 0);

     // 创建pipe，获取m_tickleFds[2]，其中m_tickleFds[0]是管道的读端，m_tickleFds[1]是管道的写端
    int rt = pipe(m_tickleFds);
    RADIXUN_ASSERT(!rt);

    // 注册pipe读句柄的可读事件，用于tickle调度协程，通过epoll_event.data.fd保存描述符
    epoll_event event;
    memset(&event , 0 , sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;//读数据+边缘触发
    event.data.fd = m_tickleFds[0];

    // 非阻塞方式，配合边缘触发
    rt = fcntl(m_tickleFds[0] , F_SETFL , O_NONBLOCK);
    RADIXUN_ASSERT(!rt);

    // 将管道的读描述符加入epoll多路复用，如果管道可读，idle中的epoll_wait会返回
    rt = epoll_ctl(m_epfd , EPOLL_CTL_ADD , m_tickleFds[0] , &event);
    RADIXUN_ASSERT(!rt);

    contextResize(32);

    // 直接开启Schedluer
    start();
}

IOManager::~IOManager(){
    RADIXUN_LOG_INFO(g_logger) << "~IOManager";
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0 ; i > m_fdContexts.size() ; i ++){
        if(m_fdContexts[i]){
            delete m_fdContexts[i];
        }
    }
}

int IOManager::addEvent(int fd , Event event , std::function<void()> cb){
    // 找到fd对应的FdContext，如果不存在，那就分配一个
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd){
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    }else {
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }
    // 这个fd的event原本有了
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(fd_ctx->events& event){
        RADIXUN_LOG_ERROR(g_logger) << "addEvent assert fd = " << fd
                                    << "event = " << event
                                    << "fd_ctx.event=" << fd_ctx->events;
        RADIXUN_ASSERT(!(fd_ctx->events& event));
    }

    // 将新的事件加入epoll_wait，使用epoll_event的私有指针存储FdContext的位置
    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;
    RADIXUN_LOG_DEBUG(g_logger) << "add-->event:" << event;
    
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        RADIXUN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << "," << fd << "," << epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return -1;
    }
    // 待执行IO事件数加1
    ++m_pendingEventCount;
    RADIXUN_LOG_DEBUG(g_logger) << "add--->count" << m_pendingEventCount;
    // 找到这个fd的event事件对应的EventContext，对其中的scheduler, cb, fiber进行赋值
    fd_ctx->events = (Event)(fd_ctx->events | event);
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    RADIXUN_LOG_DEBUG(g_logger) << "addfdevents: " << fd_ctx->events;
    RADIXUN_ASSERT(!event_ctx.scheduler
                && !event_ctx.fiber
                && !event_ctx.cb);

    // 对EventContext赋值scheduler和回调函数，如果回调函数为空，则把当前协程当成回调执行体
    event_ctx.scheduler = Scheduler::GetThis();
    if(cb) {
        event_ctx.cb.swap(cb);
    } else { //null
        event_ctx.fiber = Fiber::GetThis();
        RADIXUN_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC
                      ,"state=" << event_ctx.fiber->getState());
    }
    return 0;
}

bool IOManager::delEvent(int fd , Event event){
    // 找到fd对应的FdContext
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd){
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event)){//原本就没事件
        return false;
    }

    // 清除指定的事件，表示不关心这个事件了，如果清除之后结果为0，则从epoll_wait中删除该文件描述符
    Event new_events = (Event)(fd_ctx->events & ~event);//把事件从原本里面去掉
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd , op , fd , &epevent);
    if(rt){
        RADIXUN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << " , "
                << op << " , " << fd << " , " << epevent.events << ") : "
                << rt << "(" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    -- m_pendingEventCount;
    fd_ctx->events = new_events;
    // 重置该fd对应的event事件上下文
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

bool IOManager::cancelEvent(int fd , Event event) {
    // 找到fd对应的FdContext
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd){
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event)){//原本就没事件
        return false;
    }

    Event new_events = (Event)(fd_ctx->events & ~event);//把事件从原本里面去掉
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;
    RADIXUN_LOG_DEBUG(g_logger)<< "can: oldevent" << fd_ctx->events << ", event:" << event;
    RADIXUN_LOG_DEBUG(g_logger) << "cancel event:" << new_events;
    int rt = epoll_ctl(m_epfd , op , fd , &epevent);
    if(rt){
        RADIXUN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << " , "
                << op << " , " << fd << " , " << epevent.events << ") : "
                << rt << "(" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    // 触发已注册的事件
    RADIXUN_LOG_DEBUG(g_logger) << "cancel to tigger";   
    fd_ctx->triggerEvent(event);
    -- m_pendingEventCount;
    return true;

}

bool IOManager::cancelAll(int fd) {
    // 找到fd对应的FdContext
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {
        return false;
    }
    // 删除全部事件
    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;
    int rt = epoll_ctl(m_epfd , op , fd , &epevent);
    if(rt){
        RADIXUN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << " , "
                << op << " , " << fd << " , " << epevent.events << ") : "
                << rt << "(" << errno << ") (" << strerror(errno) << ")";
        return false;
    }
    // 触发全部已注册的事件
    if(fd_ctx->events & READ){
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE){
        fd_ctx->triggerEvent(WRITE);
        -- m_pendingEventCount;
    }
    RADIXUN_ASSERT(fd_ctx->events == 0);
    return true;
}

void IOManager::contextResize(size_t size){
    m_fdContexts.resize(size);

    for(size_t i = 0 ; i < m_fdContexts.size() ; i ++){
        if(!m_fdContexts[i]){
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle(){
    if(hasIdleThreads()){
        return ;
    }
    RADIXUN_LOG_INFO(g_logger) << "IOM: tickle";
    int rt = write(m_tickleFds[1] , "T" , 1);
    RADIXUN_ASSERT(rt == 1);
}

//stopping
bool IOManager::stopping(uint64_t& timeout) {
    timeout = getNextTimer();
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();

}

bool IOManager::stopping() {
    uint64_t timeout = 0;
    return stopping(timeout);
}

void IOManager::idle() {
    RADIXUN_LOG_DEBUG(g_logger) << "idle";
 
    epoll_event* events = new epoll_event[64]();
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;
    });
 
    while (true) {
        uint64_t next_timeout = 0;
        if(stopping(next_timeout)) {
            RADIXUN_LOG_DEBUG(g_logger) << "name=" << getName() << "idle stopping exit";
            break;
        }
        // 阻塞在epoll_wait上，等待事件发生
        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 3000;
            if(next_timeout != ~0ull) {
                next_timeout = (int)next_timeout > MAX_TIMEOUT
                                ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }
            rt = epoll_wait(m_epfd, events, 64, (int)next_timeout);
            if(rt < 0 && errno == EINTR) {
            } else {
                if(rt == 0)RADIXUN_LOG_DEBUG(g_logger) << "epoll_wait end";
                break;
            }
        } while(true);
 
        std::vector<std::function<void()> > cbs;
        listExpiredCb(cbs);
        if(!cbs.empty()) {
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }
        // 遍历所有发生的事件，根据epoll_event的私有指针找到对应的FdContext，进行事件处理
        for (int i = 0; i < rt; ++i) {
            RADIXUN_LOG_DEBUG(g_logger) << "in rt:" << rt;
            epoll_event& event = events[i];
            if(event.data.fd == m_tickleFds[0]) {
                uint8_t dummy;
                while(read(m_tickleFds[0], &dummy, 1) == 1);
                continue;
            }
            RADIXUN_LOG_DEBUG(g_logger) << "this event: " << event.events;
            // 通过epoll_event的私有指针获取FdContext
            FdContext *fd_ctx = (FdContext *)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            /**
             * EPOLLERR: 出错，比如写读端已经关闭的pipe
             * EPOLLHUP: 套接字对端关闭
             * 出现这两种事件，应该同时触发fd的读和写事件，否则有可能出现注册的事件永远执行不到的情况
             */
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT);
            }
            int real_events = NONE;
            if (event.events & EPOLLIN) {
                real_events |= READ;
            }
            if (event.events & EPOLLOUT) {
                real_events |= WRITE;
            }
 
            if ((fd_ctx->events & real_events) == NONE) {
                continue;
            }
 
            // 剔除已经发生的事件，将剩下的事件重新加入epoll_wait，
            // 如果剩下的事件为0，表示这个fd已经不需要关注了，直接从epoll中删除
            int left_events = (fd_ctx->events & ~real_events);
            int op          = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events    = EPOLLET | left_events;
            RADIXUN_LOG_DEBUG(g_logger) << "real_events--->:"<< real_events;
            RADIXUN_LOG_DEBUG(g_logger) << "left: " << left_events;

            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if (rt2) {
                RADIXUN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                           << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                                          << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }
            
            // 处理已经发生的事件，也就是让调度器调度指定的函数或协程
            if (real_events & READ) {
                RADIXUN_LOG_DEBUG(g_logger) << "dile tigger READ";
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if (real_events & WRITE) {
                RADIXUN_LOG_DEBUG(g_logger) << "dile tigger WRITE";
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        } // end for
 
        /**
         * 一旦处理完所有的事件，idle协程yield，这样可以让调度协程(Scheduler::run)重新检查是否有新任务要调度
         * 上面triggerEvent实际也只是把对应的fiber重新加入调度，要执行的话还要等idle协程退出
         */
        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr   = cur.get();
        cur.reset();
        RADIXUN_LOG_DEBUG(g_logger) << "swapout";
        raw_ptr->swapOut();
    } // end while(true)
}

void IOManager::onTimerInsertedAtFront() {
    tickle();
}

}