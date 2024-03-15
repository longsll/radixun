#include "thread.h"
#include "log.h"
#include "util.h"
#include <iostream>

namespace radixun{

//当前线程的Thread指针
static thread_local Thread* t_thread = nullptr;
//当前线程名称
static thread_local std::string t_thread_name = "NUKNOW";

static radixun::Logger::ptr g_logger = RADIXUN_LOG_NAME("system");

Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}

void Thread::SetName(const std::string& name){
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(std::function<void()>cb , const std::string& name)
    :m_cb(cb),m_name(name){
    if(name.empty()){
        m_name = "NUKNOW";
    }
    int rt = pthread_create(&m_thread , nullptr , &Thread::run , this);
    if(rt){
        RADIXUN_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
            << " name=" << name;
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}

Thread::~Thread() {
    if(m_thread) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if(m_thread) {
        int rt = pthread_join(m_thread , nullptr);
        if(rt){
            RADIXUN_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

void* Thread::run(void* arg){
    //传进来的参数转化
    Thread* thread = (Thread*) arg;
    //设置参数,当前执行线程
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = radixun::GetThreadId();
    //设置线程名称
    pthread_setname_np(pthread_self() , thread->m_name.substr(0,15).c_str());

    //防止智能指针引用不被释放
    std::function<void()>cb;
    cb.swap(thread->m_cb);

    thread->m_semaphore.notify();

    cb();
    return 0;
}

}