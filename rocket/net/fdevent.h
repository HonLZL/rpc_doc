#ifndef ROCKET_NET_FDEVENT_H
#define ROCKET_NET_FDEVENT_H

#include <sys/epoll.h>
#include <functional>

namespace rocket {
class FdEvent {
   public:
    enum TriggerEvent {
        IN_EVENT = EPOLLIN;    // EPOLLIN 用于指示关联的文件描述符上有数据可读取
        OUT_EVENT = EPOLLOUT;  // 可些事件
    }
    FdEvent(int fd);
    ~FdEvent();

    std::function<void()> handler(TriggerEvent event_type);
    void listen(TriggerEvent event_type, std::function<void()> callback);
    int getFd() const {
        return m_fd;
    }
    epoll_event getEpollEvent() {
        return m_listen_events;
    }

   protected:
    int m_fd{-1};
    std::function<void()> m_read_callback;
    std::function<void()> m_write_callback;
};
}  // namespace rocket