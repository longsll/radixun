#ifndef __RADIXUN__IOMANAGER_H__
#define __RADIXUN__IOMANAGER_H__

#include "scheduler.h"
#include "timer.h"

namespace radixun{

class IOManager :public Scheduler , public TimerManager{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    //IO事件
    enum Event{
        // 无事件
        NONE = 0x00,
        // 读事件(EPOLLIN)
        READ = 0x01,
        // 写事件(EPOLLOUT)
        WRITE = 0x4,
    };

private:
    // Socket事件上线文类(描述符-事件类型-回调函数)
    struct FdContext{
        typedef Mutex MutexType;
        struct EventContext {
            Scheduler* scheduler = nullptr;
            Fiber::ptr fiber;
            std::function<void()> cb;
        };

        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);

        EventContext read;
        EventContext write;
        int fd = 0;
        Event events = NONE;
        MutexType mutex;
    };

public :
    IOManager(size_t threads = 1 , bool use_caller = true , const std::string& name = "");
    ~IOManager();

    //添加事件(添加成功返回0,失败返回-1)
    int addEvent(int fd , Event event , std::function<void()> cb = nullptr);
    //删除事件(不会触发事件)
    bool delEvent(int fd , Event event);
    //取消事件(触发一次回调事件)
    bool cancelEvent(int fd , Event event);
    bool cancelAll(int fd);
    // 返回当前的IOManager
    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertedAtFront() override;
    void contextResize(size_t size);
    bool stopping(uint64_t& timeout);
private:
    // epoll 文件句柄
    int m_epfd = 0;
    // pipe 文件句柄，fd[0]读端，fd[1]写端
    int m_tickleFds[2];
    // 当前等待执行的IO事件数量
    std::atomic<size_t> m_pendingEventCount = {0};
    // IOManager的Mutex
    RWMutexType m_mutex;
    // socket事件上下文的容器
    std::vector<FdContext*> m_fdContexts;
};

}


#endif