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
        //事件上线文类
        struct EventContext {
            // 事件执行的调度器
            Scheduler* scheduler = nullptr;
            // 事件协程
            Fiber::ptr fiber;
            // 事件的回调函数
            std::function<void()> cb;
        };

        //获取事件上下文类
        EventContext& getContext(Event event);
        //重置事件上下文
        void resetContext(EventContext& ctx);
        //触发事件
        void triggerEvent(Event event);

        // 读事件上下文
        EventContext read;
        // 写事件上下文
        EventContext write;
        // 事件关联的句柄
        int fd = 0;
        // 该fd添加了哪些事件的回调函数，或者说该fd关心哪些事件
        Event events = NONE;
        // 事件的Mutex
        MutexType mutex;
    };

public :
    /**
     * @brief 构造函数
     * @param[in] threads 线程数量
     * @param[in] use_caller 是否将调用线程包含进去
     * @param[in] name 调度器的名称
     */
    IOManager(size_t threads = 1 , bool use_caller = true , const std::string& name = "");

    /**
     * @brief 析构函数
     */
    ~IOManager();

    /**
     * @brief 添加事件
     * @details fd描述符发生了event事件时执行cb函数
     * @param[in] fd socket句柄
     * @param[in] event 事件类型
     * @param[in] cb 事件回调函数，如果为空，则默认把当前协程作为回调执行体
     * @return 添加成功返回0,失败返回-1
     */
    int addEvent(int fd , Event event , std::function<void()> cb = nullptr);

    /**
     * @brief 删除事件
     * @param[in] fd socket句柄
     * @param[in] event 事件类型
     * @attention 不会触发事件
     * @return 是否删除成功
     */
    bool delEvent(int fd , Event event);

    /**
     * @brief 取消事件
     * @param[in] fd socket句柄
     * @param[in] event 事件类型
     * @attention 如果该事件被注册过回调，那就触发一次回调事件
     * @return 是否删除成功
     */
    bool cancelEvent(int fd , Event event);

    /**
     * @brief 取消所有事件
     * @details 所有被注册的回调事件在cancel之前都会被执行一次
     * @param[in] fd socket句柄
     * @return 是否删除成功
     */
    bool cancelAll(int fd);

    /**
     * @brief 返回当前的IOManager
     */
    static IOManager* GetThis();

protected:
    /**
     * @brief 通知调度器有任务要调度
     * @details 写pipe让idle协程从epoll_wait退出，待idle协程yield之后Scheduler::run就可以调度其他任务
     * 如果当前没有空闲调度线程，那就没必要发通知
     */
    void tickle() override;

    bool stopping() override;
    /**
     * @brief idle协程
     * @details 对于IO协程调度来说，应阻塞在等待IO事件上，idle退出的时机是epoll_wait返回，对应的操作是tickle或注册的IO事件就绪
     * 调度器无调度任务时会阻塞idle协程上，对IO调度器而言，idle状态应该关注两件事，一是有没有新的调度任务，对应Schduler::schedule()，
     * 如果有新的调度任务，那应该立即退出idle状态，并执行对应的任务；二是关注当前注册的所有IO事件有没有触发，如果有触发，那么应该执行
     * IO事件对应的回调函数
     */
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