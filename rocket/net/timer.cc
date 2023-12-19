#include <string.h>
#include <sys/timerfd.h>

#include "../common/log.h"
#include "../common/util.h"
#include "../common/mutex.h"
#include "timer.h"

namespace rocket {
Timer::Timer()
    : FdEvent() {
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    DEBUGLOG("timer fd = %d \n", m_fd);

    // 把fd刻度事件放到了 eventloop 上监听
    listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
}

Timer::~Timer() {
}

void Timer::onTimer() {
    DEBUGLOG("ontimer");
    // 处理 fd,把可读事件清楚掉,不然下次 epoll 还会返回,ET模式
    // 处理缓冲区数据, 防止下一次继续触发可读事件
    char buf[8];
    while (1) {
        if (read(m_fd, buf, 8) == -1 && errno == EAGAIN) {
            break;
        }
    }
    // 执行定时任务
    int64_t now = getNowMs();
    std::vector<TimerEvent::s_ptr> tmps;
    std::vector<std::pair<int64_t, std::function<void()>>> tasks;

    ScopeMutex<Mutex> lock(m_mutex);

    // 寻找要执行的任务
    auto it = m_pending_events.begin();
    for (it = m_pending_events.begin(); it != m_pending_events.end(); it++) {
        if ((*it).first <= now) {
            if (!(*it).second->isCancled()) {
                tmps.push_back((*it).second);
                tasks.push_back(std::make_pair((*it).second->getArriveTime(), (*it).second->getCallBack()));
            }
        } else {
            break;
        }
    }
    // 删除上述任务
    m_pending_events.erase(m_pending_events.begin(), it);
    lock.unlock();

    // 需要把重复的 Event 再次添加进去
    for (auto i = tmps.begin(); i != tmps.end(); i++) {
        if ((*i)->isRepeated()) {
            // 调整 arriveTime
            (*i)->resetArriveTime();
            addTimerEvent(*i);
        }
    }

    resetArriveTime();
    for (auto i : tasks) {
        if (i.second) {
            i.second();
        }
    }
}

void Timer::resetArriveTime() {
    ScopeMutex<Mutex> lock(m_mutex);
    auto tmp = m_pending_events;
    lock.unlock();
    if (tmp.size() == 0) {
        return;
    }
    int64_t now = getNowMs();
    auto it = tmp.begin();
    int64_t inteval = 0;
    if (it->second->getArriveTime() > now) {
        inteval = it->second->getArriveTime() - now;
    } else {
        inteval = 100;
    }
    timespec ts;
    memset(&ts, 0, sizeof(ts)); 
    ts.tv_sec = inteval / 1000;
    ts.tv_nsec = (inteval % 100) * 1000;

    itimerspec value;
    memset(&value, 0, sizeof(value));
    value.it_value = ts;

    // fd 会在指定的时间,触发可读事件
    int rt = timerfd_settime(m_fd, 0, &value, nullptr);
    if (rt != 0) {
        ERRORLOG("timefd set time error, errno=%d, error=%s", errno, strerror(errno));
    }
    DEBUGLOG("timer reset to %lld", now + inteval);
}

void Timer::addTimerEvent(TimerEvent::s_ptr event) {
    bool is_reset_timerfd = false;

    ScopeMutex<Mutex> lock(m_mutex);

    if (m_pending_events.empty()) {  // 当前事件为空,需要设置
        is_reset_timerfd = true;
    } else {  //
        auto it = m_pending_events.begin();
        // 要插入的定时任务, 判断 是否 大于 当前任务队列的定时时间,说明,要修改定时触发时间
        // 不修改的话,现在的执行时间还是地一个定时任务 (it*) 执行的时间,
        // 不设置的话,默认时间是第一个早于新插入的,新插入的到时间也不会触发
        if ((*it).second->getArriveTime() > event->getArriveTime()) {
            is_reset_timerfd = true;
        }
    }
    m_pending_events.emplace(event->getArriveTime(), event);
    lock.unlock();

    if (is_reset_timerfd) {
        resetArriveTime();
    }
}

void Timer::deleteTimerEvent(TimerEvent::s_ptr event) {
    event->setCancled(true);

    ScopeMutex<Mutex> lock(m_mutex);

    // 寻找一个时间和 event 一样的,删除
    auto begin = m_pending_events.lower_bound(event->getArriveTime());
    auto end = m_pending_events.upper_bound(event->getArriveTime());
    auto it = begin;
    for (it = begin; it != end; it++) {
        if (it->second == event) {
            break;
        }
    }
    if (it != end) {
        m_pending_events.erase(it);
    }
    lock.unlock();
    ERRORLOG("successfully delete TimeEvent at arrive time %lld", event->getArriveTime());
}

}  // namespace rocket