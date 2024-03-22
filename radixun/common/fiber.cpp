#include "fiber.h"
#include "log.h"
#include "macro.h"
#include "config.h"
#include "scheduler.h"
#include <atomic>

namespace radixun{

static Logger::ptr g_logger = RADIXUN_LOG_NAME("system");

/// 全局静态变量，用于生成协程id
static std::atomic<uint64_t> s_fiber_id {0};
/// 全局静态变量，用于统计当前的协程数
static std::atomic<uint64_t> s_fiber_count {0};
/// 线程局部变量，当前线程正在运行的协程
static thread_local Fiber* t_fiber = nullptr;
/// 线程局部变量，当前线程的主协程，切换到这个协程，就相当于切换到了主线程中运行，智能指针形式
static thread_local Fiber::ptr t_threadFiber = nullptr;
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size" , 128*1024 , "fiber stack size");

//malloc封装
class MallocStackAllocator{
public:
    static void* Alloc(size_t size){return malloc(size);}
    static void Delloc(void* vp , size_t size){return free(vp);}
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
    if(t_fiber){return t_fiber ->getId();}
    return 0;
}

Fiber::Fiber() {
    m_state =EXEC;
    SetThis(this);
    // 获取(创建)当前的上下文
    if(getcontext(&m_ctx)){
        RADIXUN_ASSERT2(false , "getcontext");
    }
    ++s_fiber_count;
    RADIXUN_LOG_DEBUG(g_logger) << "Fiber::fiber()";
}

Fiber::Fiber(std::function<void()> cb , size_t stacksize , bool use_caller)
:m_id(++s_fiber_id),m_cb(cb)
{
    ++ s_fiber_count;
    //为0使用config
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    // 获取(创建)当前的上下文
    if(getcontext(&m_ctx)){
        RADIXUN_ASSERT2(false , "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if(use_caller){
        makecontext(&m_ctx , &Fiber::CallerMainFunc , 0);
    }else{
        makecontext(&m_ctx , &Fiber::MainFunc , 0);
    }

    RADIXUN_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}

Fiber::~Fiber() {
    -- s_fiber_count;
    if(m_stack) {
        RADIXUN_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
        StackAllocator::Delloc(m_stack , m_stacksize);
    }else {
        RADIXUN_ASSERT(!m_cb);
        RADIXUN_ASSERT(m_state == EXEC);
        //若是当前执行的协程则将当前运行的协程置空
        Fiber* cur = t_fiber;
        if(cur == this){
            SetThis(nullptr);
        }
    }
    RADIXUN_LOG_DEBUG(g_logger) << "Fiber::~Fiber id= " << m_id;
}


void Fiber::reset(std::function<void()> cb) {
    RADIXUN_ASSERT(m_stack);
    RADIXUN_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    m_cb = cb;
    if(getcontext(&m_ctx)) {
        RADIXUN_ASSERT2(false , "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx , &Fiber::MainFunc , 0);
    m_state = INIT;
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx , &m_ctx)){
        RADIXUN_ASSERT2(false , "call:swapcontext");
    }
}

void Fiber::back() {
    SetThis(this);
    if(swapcontext(&m_ctx , &t_threadFiber->m_ctx)) {
        RADIXUN_ASSERT2(false , "back:swapcontext");
    }
}

void Fiber::swapIn() {
    SetThis(this);
    RADIXUN_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        RADIXUN_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        RADIXUN_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber* f){
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if(t_fiber){
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fber(new Fiber);
    RADIXUN_ASSERT(t_fiber == main_fber.get());
    t_threadFiber = main_fber;
    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    RADIXUN_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    RADIXUN_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    RADIXUN_ASSERT(cur);

    try{
        //执行cb
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }catch (std::exception& ex) {
        RADIXUN_LOG_ERROR(g_logger)<< "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << radixun::BacktraceToString();
    }catch(...) {
        cur->m_state = EXCEPT;
        RADIXUN_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << radixun::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
    RADIXUN_ASSERT2(false, "never reach fiber_id= "+ std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    RADIXUN_ASSERT(cur);

    try{
        //执行cb
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }catch (std::exception& ex) {
        RADIXUN_LOG_ERROR(g_logger)<< "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << radixun::BacktraceToString();
    }catch(...) {
        cur->m_state = EXCEPT;
        RADIXUN_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << radixun::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    RADIXUN_ASSERT2(false, "never reach fiber_id= "+ std::to_string(raw_ptr->getId()));
}

}