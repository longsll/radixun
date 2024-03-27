#ifndef __RADIXUN__TIMER_H__
#define __RADIXUN__TIMER_H__

#include<memory>
#include<vector>
#include<set>
#include "thread.h"

namespace radixun{

class TimerManager;
class Timer : public std::enable_shared_from_this<Timer>{
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;

    //取消定时器
    bool cancel();
    //刷新设置定时器的执行时间
    bool refresh();
    bool reset(uint64_t ms , bool from_now);

private:

    Timer(uint64_t ms , std::function<void()> cb,bool recurring , TimerManager* manager);
    Timer(uint64_t next);
    //定时器比较仿函数
    struct Comparator{
        bool operator()(const Timer::ptr& lhs , const Timer::ptr& rhs)const;
    };

private:
    // 是否循环定时器
    bool m_recurring = false;
    // 执行周期
    uint64_t m_ms = 0;
    // 精确的执行时间
    uint64_t m_next = 0;
    // 回调函数
    std::function<void()> m_cb;
    // 定时器管理器
    TimerManager* m_manager = nullptr;
};
class TimerManager{

friend class Timer;
public:
    typedef RWMutex RWMutexType;

    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms , std::function<void() >cb, bool recurring = false);
    Timer::ptr addConditionTimer(uint64_t ms , std::function<void()> cb, std::weak_ptr<void> weak_cond , bool recurring = false);
    uint64_t getNextTimer();
    //获取需要执行的定时器的回调函数列表
    void listExpiredCb(std::vector<std::function<void()>> & cbs);
    //是否有定时器
    bool hasTimer();

protected:
    virtual void onTimerInsertedAtFront() = 0;
    //将定时器添加到管理器中
    void addTimer(Timer::ptr val , RWMutexType::WriteLock& lock);
private:
    RWMutexType m_mutex;
    // 定时器集合
    std::set<Timer::ptr , Timer::Comparator> m_timers;
    // 是否触发onTimerInsertedAtFront
    bool m_tickled = false;
    // 上次执行时间
    uint64_t m_previouseTime = 0;
};
}

#endif