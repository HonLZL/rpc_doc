#include "fd_event.h"
#include <fcntl.h>
#include <string.h>

#include "../common/log.h"

namespace rocket {
FdEvent::FdEvent(int fd)
    : m_fd(fd) {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}
FdEvent::FdEvent() {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}
FdEvent::~FdEvent() {
}
std::function<void()> FdEvent::handler(TriggerEvent event_type) {
    if (event_type == TriggerEvent::IN_EVENT) {
        return m_read_callback;
    } else if (event_type == TriggerEvent::OUT_EVENT){
        return m_write_callback;
    } else if (event_type == TriggerEvent::ERROR_EVENT){
        return m_error_callback;
    } else {
        return nullptr;
    }
}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback, std::function<void()> error_callback /* = nullptr */) {
    if (event_type == TriggerEvent::IN_EVENT) {
        // 使用按位或运算符 |= 来给 m_listen_events.events 成员添加 EPOLLIN(可读) 标志位。
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
    } else {
        m_listen_events.events |= EPOLLOUT;
        m_write_callback = callback;
    }
    if(m_error_callback != nullptr) {
        m_error_callback = error_callback;
    } else {
        m_error_callback = nullptr;
    }
    m_listen_events.data.ptr = this;
}

void FdEvent::cancleListen(TriggerEvent event_type) {
    if (event_type == TriggerEvent::IN_EVENT) {
        // 使用按位或运算符 |= 来给 m_listen_events.events 成员添加 EPOLLIN(可读) 标志位。
        m_listen_events.events &= (~EPOLLIN);
    } else {
        // m_listen_events.events &= (EPOLLOUT);
        m_listen_events.events &= (~EPOLLOUT);
    }
}

// fcntl 函数改变文件描述符标志
void FdEvent::setNonBlock() {
    // 将文件描述符设为非阻塞模式
    int flag = fcntl(m_fd, F_GETFL, 0);
    if (flag & O_NONBLOCK) {
        return;
    }
    fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
}
}  // namespace rocket