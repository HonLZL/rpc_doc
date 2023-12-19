#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include <pthread.h>
#include <functional>
#include <queue>
#include <set>

#include "../common/mutex.h"
#include "fd_event.h"
#include "wakeup_fd_event.h"

namespace rocket {
class EventLoop {
   public:
    EventLoop();
    ~EventLoop();

    void loop();
    void wakeup();
    void stop();

    void addEpollEvent(FdEvent* event);
    void deleteEpollEvent(FdEvent* event);

    bool isInLoopThread();

    // 把任务先添加到一个 pending 队列里面,等线程从 epoll_wait返回,不是由其他线程去执行
    void addTask(std::function<void()> cb, bool is_wakeup = false);

   private:
    void dealWakeup();

    void initWakeUpFdEvent();

   private:
    pid_t m_thread_id{0};
    int m_epoll_fd{0};
    int m_wakeup_fd{0};

    bool m_stop_flag{false};

    WakeUpFdEvent* m_wakeup_fd_event{nullptr};

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks;  // void 类型的函数队列

    Mutex m_mutex;
};
}  // namespace rocket

#endif