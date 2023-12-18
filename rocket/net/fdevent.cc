#include "fdevent.h"
#include "../common/log.h"

namespace rocket {
FdEvent::FdEvent(int fd)
    : m_fd(fd) {
}
FdEvent : ~FdEvent() {
}
std::function<void()>  FdEvent::handler(TriggerEvent event_type) {
    if (event == TriggerEvent::INEVENT) {
        return m_read_callback;
    } else {
        return m_write_callback;
    }
}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback) {
    if (event_type == TriggerEvent::INEVENT) {
        // 使用按位或运算符 |= 来给 m_listen_events.events 成员添加 EPOLLIN(可读) 标志位。
        m_listen_events.events |= EPOLLIN;
    } else {
        m_listen_events.events |= EPOLLOUT;
    }
    m_read_callback = callback;
    m_listen_events.data.ptr = this;
}
}  // namespace rocket