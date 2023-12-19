#ifndef ROCKET_NET_TIMER_H
#define ROCKET_NET_TIMER_H

#include <map>

#include "../common/mutex.h"
#include "fd_event.h"
#include "timer_event.h"

namespace rocket {
class Timer : public FdEvent {
   public:
    Timer();
    ~Timer();

    void addTimerEvent(TimerEvent::s_ptr event);
    void deleteTimerEvent(TimerEvent::s_ptr event);

    void onTimer();

   private:
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;

    Mutex m_mutex; //如果追求更好的性能,可以替换为读写锁

    void resetArriveTime();
};

}  // namespace rocker

#endif