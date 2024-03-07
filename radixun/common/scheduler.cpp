#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"


namespace radixun {

static radixun::Logger::ptr g_logger = RADIXUN_LOG_NAME("system");

/// 当前线程的调度器，同一个调度器下的所有线程指同同一个调度器实例
static thread_local Scheduler* t_scheduler = nullptr;
/// 当前线程的调度协程，每个线程都独有一份，包括caller线程
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads , bool use_caller , const std::string& name)
    :m_name(name)
{
    // RADIXUN_LOG_DEBUG(g_logger) << "Scheduler()";
    RADIXUN_ASSERT(threads > 0);

    if(use_caller){
        radixun::Fiber::GetThis();
        -- threads;

        RADIXUN_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run , this) , 0 , true));
        radixun::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = radixun::GetThreadId();
        m_threadIds.push_back(m_rootThread);

    }else{
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    RADIXUN_LOG_INFO(g_logger) << "~Scheduler";
    RADIXUN_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis(){
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber(){
    return t_scheduler_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping){
        return ;
    }
    m_stopping = false;
    RADIXUN_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0 ; i < m_threadCount ; i++){
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run , this)
        , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    // RADIXUN_LOG_DEBUG(g_logger) << "start()";
    lock.unlock();
}

void Scheduler::stop() {
    RADIXUN_LOG_DEBUG(g_logger) << "stop";
    m_autoStop = true;
    if(m_rootFiber && m_threadCount == 0
    && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT)){
        RADIXUN_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping()){
            return ;
        }
    }
    /// 如果use caller，那只能由caller线程发起stop
    if(m_rootThread != -1){
        RADIXUN_ASSERT(GetThis() == this);
    }else{
        RADIXUN_ASSERT(GetThis() != this);
    }
    m_stopping = true;
    for(size_t i = 0 ; i < m_threadCount ; i ++){
        // RADIXUN_LOG_DEBUG(g_logger) << "stop to tick other";
        tickle();
    }

    if(m_rootFiber){
        tickle();
    }
    /// 在use caller情况下，调度器协程结束时，应该返回caller协程
    if(m_rootFiber){
        if(!stopping()){
            RADIXUN_LOG_DEBUG(g_logger) << "tocall";
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }
    for(auto& i :thrs){
        i->join();
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    // RADIXUN_LOG_INFO(g_logger) << "run";
    set_hook_enable(true);
    setThis();
    if(radixun::GetThreadId() != m_rootThread){
        t_scheduler_fiber = Fiber::GetThis().get();//创建调度协程
    }
    //当任务队列为空时，代码会进idle协程
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle ,this)));
    //要执行的任务协程
    Fiber::ptr cb_fiber;
    //调度任务
    FiberAndThread ft;

    while(true){
        ft.reset();
        //是否有任务到来
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()){
                // 指定了调度线程，但不是在当前线程上调度，标记一下需要通知其他线程进行调度，然后跳过这个任务，继续下一个
                if(it-> thread != -1 && it->thread != radixun::GetThreadId()){
                    ++it;
                    tickle_me = true;
                    continue;
                }
                // 找到一个未指定线程，或是指定了当前线程的任务
                RADIXUN_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }
                //ft为要执行的任务
                ft = *it;
                m_fibers.erase(it);
                ++m_activeThreadCount;
                is_active = true;
                // RADIXUN_LOG_DEBUG(g_logger) << "add" << m_activeThreadCount;
                break;
            }
        }

        if(tickle_me){
            // RADIXUN_LOG_DEBUG(g_logger) << "run to tick other";
            tickle();
        }
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)){
            // resume协程，resume返回时，协程要么执行完了，要么半路yield了，总之这个任务就算完成了，活跃线程数减一
            ft.fiber->swapIn();
            --m_activeThreadCount;
            // RADIXUN_LOG_DEBUG(g_logger) << "sub1" << m_activeThreadCount;
            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            }else if(ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT){
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        }else if(ft.cb) {
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            }else{
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            // RADIXUN_LOG_DEBUG(g_logger) << "sub2" << m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            }else if(cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM){
                cb_fiber->reset(nullptr);
            }else{
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        }else{
            // RADIXUN_LOG_DEBUG(g_logger) << "go idle";
            // 进到这个分支情况一定是任务队列空了，调度idle协程即可
            // RADIXUN_LOG_DEBUG(g_logger) << "run empty";
            if(is_active) {
                -- m_activeThreadCount;
                // RADIXUN_LOG_DEBUG(g_logger) << "sub3" << m_activeThreadCount;
                continue;
            }
            // 如果调度器没有调度任务，那么idle协程会不停地resume/yield，不会结束，如果idle协程结束了，那一定是调度器停止了
            if(idle_fiber->getState() == Fiber::TERM){
                RADIXUN_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++ m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT){
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}

void Scheduler::tickle() {
    // RADIXUN_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    RADIXUN_LOG_INFO(g_logger) << "idle";
    while(!stopping()){
        radixun::Fiber::YieldToHold();
    }
}

};