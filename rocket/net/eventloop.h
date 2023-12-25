#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include <pthread.h>
#include <functional>
#include <queue>
#include <set>

#include "../common/mutex.h"
#include "fd_event.h"
#include "timer.h"
#include "wakeup_fd_event.h"

namespace rocket {
class EventLoop {
   public:
    EventLoop();
    ~EventLoop();

    void loop();
    // void loop() {
    //     while (!stop) {
    //         foreach (task in tasks) {
    //             task();
    //         }
    //         // 1.取得下次定时任务的时间，与设定time_out去较大值，即若下次定时任务时间超过1s就取下次定时任务时间为超时时间，否则取1s
    //         int time_out = Max(1000, getNextTimerCallback());
    //         // 2.调用Epoll等待事件发生，超时时间为上述的time_out
    //         int rt = epoll_wait(epfd, fds, ...., time_out);
    //         if (rt < 0) {
    //             // epoll调用失败。。
    //         } else {
    //             if (rt > 0) {
    //                 foreach (fd in fds) {
    //                     // 添加待执行任务到执行队列
    //                     tasks.push(fd);
    //                 }
    //             }
    //         }
    //     }
    // }
    void wakeup();
    void stop();

    void addEpollEvent(FdEvent* event);
    void deleteEpollEvent(FdEvent* event);

    bool isInLoopThread();

    // 把任务先添加到一个 pending 队列里面,等线程从 epoll_wait返回,不是由其他线程去执行
    void addTask(std::function<void()> cb, bool is_wakeup = false);

    void addTimerEvent(TimerEvent::s_ptr event);

    static EventLoop* GetCurrentEventLoop();

    bool isLooping();

   private:
    void dealWakeup();

    void initWakeUpFdEvent();

    void initTimer();

   private:
    pid_t m_thread_id{0};
    int m_epoll_fd{0};
    int m_wakeup_fd{0};

    bool m_stop_flag{false};

    WakeUpFdEvent* m_wakeup_fd_event{nullptr};

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks;  // void 类型的函数队列

    Mutex m_mutex;

    Timer* m_timer{nullptr};

    bool m_is_looping {false};
};
}  // namespace rocket

#endif