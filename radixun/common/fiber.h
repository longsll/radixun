#ifndef __RADIXUN_FIBER_H__
#define __RADIXUN_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>
#include "thread.h"

namespace radixun{

//协程调度器
class Scheduler;

class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;

public:
    typedef std::shared_ptr<Fiber> ptr;

    //协程状态
    enum State{
        /// 初始化状态
        INIT,
        /// 暂停状态
        HOLD,
        /// 执行中状态
        EXEC,
        /// 结束状态
        TERM,
        /// 可执行状态
        READY,
        /// 异常状态
        EXCEPT
    };

private:
    //无参构造函数只用于创建线程的第一个协程，也就是线程主函数对应的协程，
    //这个协程只能由GetThis()方法调用，所以定义成私有方法
    Fiber();

public:

    Fiber(std::function<void()>cb , size_t stacksize = 0 , bool use_caller = false);
    ~Fiber();
    // 协程的重置，重置协程就是重复利用已结束的协程，复用其栈空间，创建新协程
    void reset(std::function<void()> cb);
    //将当前协程切换到运行状态(与调度协程)
    //执行的为当前线程的调度协程
    void swapIn();
    //将当前协程切换到后台(与调度协程)
    void swapOut();
    //将当前线程切换到执行状态(与主协程)
    void call();
    //将当前线程切换到后台(与主协程)
    void back();
    //返回协程id
    uint64_t getId() const {return m_id;}
    //返回协程状态
    State getState() const {return m_state;}

public:
    //设置当前线程的运行协程
    static void SetThis(Fiber* f);
    //返回当前所在的协程,如果没有将创建一个
    static Fiber::ptr GetThis();
    //将当前协程切换到后台,并设置为READY状态
    static void YieldToReady();
    //将当前协程切换到后台,并设置为HOLD状态
    static void YieldToHold();
    //返回当前协程的总数量
    static uint64_t TotalFibers();
    //协程执行函数
    //执行完成返回到线程主协程
    static void MainFunc();
    //协程执行函数
    //执行完成返回到线程调度协程
    static void CallerMainFunc();
    //获取当前协程的id
    static uint64_t GetFiberId();

private:
    //协程id
    uint64_t m_id = 0;
    //协程运行栈大小
    uint32_t m_stacksize = 0;
    //协程状态
    State m_state = INIT;
    //协程上下文
    ucontext_t m_ctx;
    //协程运行栈指针
    void* m_stack = nullptr;
    //协程运行函数
    std::function<void()> m_cb;
};

}


#endif