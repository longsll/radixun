#ifndef RADIXUN_THREAD_H__
#define RADIXUN_THREAD_H__

#include<thread>
#include<functional>
#include<memory>
#include<pthread.h>
#include<semaphore.h>
#include<stdint.h>
#include<atomic>
#include<string>

#include"noncopyable.h"

namespace radixun{

//信号量
class Semaphore : Noncopyable {
public:
    /**
     * @brief 构造函数
     * @param[in] count 信号量值的大小
     */
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void notify();
private:

    sem_t m_semaphore;

};

//局部锁的模板实现
template<class T>
class ScopedLockImpl {
public:
    ScopedLockImpl (T& mutex):m_mutex(mutex){
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl(){
        unlock();
    }

    void lock(){
        if(!m_locked){
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    //mutex
    T& m_mutex;
    //是否已上锁
    bool m_locked;

};

//局部读锁模板实现
template<class T>
class ReadScopedLockImpl{
public:

    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }
    ~ReadScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    //mutex
    T& m_mutex;
    //是否已上锁
    bool m_locked;
};


//局部写锁模板实现
template<class T>
class WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }
    ~WriteScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// Mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};

//互斥量
class Mutex : Noncopyable {
public:
    /// 局部锁
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex(){
        pthread_mutex_init(& m_mutex , nullptr);
    }
    ~Mutex(){
        pthread_mutex_destroy(&m_mutex);
    }
    void lock(){
        pthread_mutex_lock(&m_mutex);
    }
    void unlock(){
        pthread_mutex_unlock(&m_mutex);
    }
private:
    //mutex
    pthread_mutex_t m_mutex;
};


//空锁(用于调试)
class NullMutex : Noncopyable{
public:
    /// 局部锁
    typedef ScopedLockImpl<NullMutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

//读写互斥量
class RWMutex : Noncopyable {
public:
    /// 局部读锁
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    /// 局部写锁
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex(){
        pthread_rwlock_init(&m_lock , nullptr);
    }
    ~RWMutex(){
        pthread_rwlock_destroy(&m_lock);
    }
    void rdlock(){
        pthread_rwlock_rdlock(&m_lock);
    }
    void wrlock(){
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock(){
        pthread_rwlock_unlock(&m_lock);
    }

private:
    pthread_rwlock_t m_lock;
};



//空读写锁(用于调试)
class NullRWMutex : Noncopyable {
public:
    /// 局部读锁
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    /// 局部写锁
    typedef WriteScopedLockImpl<NullMutex> WriteLock;
    NullRWMutex() {}
    ~NullRWMutex() {}
    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};


//自旋锁
class Spinlock : Noncopyable {
public:
    /// 局部锁
    typedef ScopedLockImpl<Spinlock> Lock;

    Spinlock(){
        pthread_spin_init(&m_mutex , 0);
    }
    ~Spinlock(){
        pthread_spin_destroy(&m_mutex);
    }
    void lock(){
        pthread_spin_lock(&m_mutex);
    }
    void unlock(){
        pthread_spin_unlock(&m_mutex);
    }

private:
    pthread_spinlock_t m_mutex;
};


//原子锁
class CASLock : Noncopyable {
public:
    typedef ScopedLockImpl<CASLock> Lock;

    CASLock(){
        m_mutex.clear();
    }
    ~CASLock(){

    }
    void lock(){
        while(std::atomic_flag_test_and_set_explicit(&m_mutex , std::memory_order_acquire));
    }
    void unlick(){
        std::atomic_flag_clear_explicit(&m_mutex , std::memory_order_release);
    }
private:
    // 原子状态
    volatile std::atomic_flag m_mutex;
};

//线程类
class Thread : Noncopyable {
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()>cb , const std::string& name);
    ~Thread();

    //线程ID
    pid_t getId() const {return m_id;}
    //线程名称
    const std::string& getName() const {return m_name;}
    //等待线程执行完成
    void join();
    //获取当前的线程指针
    static Thread* GetThis();
    //获取当前的线程名称
    static const std::string& GetName();
    //设置当前线程名称
    static void SetName(const std::string& name);

private:
    //线程执行函数
    static void* run(void* arg);

private:
    //线程id
    pid_t m_id = -1;
    //线程结构
    pthread_t m_thread = 0;
    //线程执行函数
    std::function<void()> m_cb;
    //线程名称
    std::string m_name;
    //信号量
    Semaphore m_semaphore;

};

}

#endif