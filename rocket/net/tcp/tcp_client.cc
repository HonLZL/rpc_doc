#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../../common/log.h"
#include "../eventloop.h"
#include "../fd_event_group.h"
#include "tcp_client.h"
#include "tcp_connection.h"

namespace rocket {
TcpClient::TcpClient(NetAddr::s_ptr peer_addr)
    : m_peer_addr(peer_addr) {
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);

    if (m_fd < 0) {
        ERRORLOG("TcpClient::TcpClient() error, failed to create fa");
        return;
    }
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);

    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr);
    m_connection->setConnectionType(TcpConnectionByClient);
}
TcpClient::~TcpClient() {
    if (m_fd > 0) {
        close(m_fd);
    }
}

// 异步进行connect,如果connect成功, done 会被执行
void TcpClient::connect(std::function<void()> done) {
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if (rt == 0) {
        DEBUGLOG("connect [%s] successfully", m_peer_addr->toString().c_str());
        if (done) {
            done();
        }
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // EINPROGRESS 错误码表明某些套接字操作（通常是非阻塞连接）仍在进行中，尚未完成
            // epoll 监听可写事件,然后判断错误码
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]() {
                int error = 0;
                socklen_t error_len = sizeof(error);
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                if (error = 0) {
                    DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                    if (done) {
                        done();
                    }
                } else {
                    ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));
                }
                // 连接完要去掉可写事件的监听,不然会一直触发
                m_fd_event->cancleListen(FdEvent::OUT_EVENT);
                m_event_loop->addEpollEvent(m_fd_event);
            });
            m_event_loop->addEpollEvent(m_fd_event);
            if (!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }
        } else {
            ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));
        }
    }
}

// 异步发送 Message, 字符串 或 RPC 协议,发送成功,会调用 done 函数,函数的入参就是 message 对象
void TcpClient::writeMessage(AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)> done) {
}

void TcpClient::readMessage(AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)> done) {
}

}  // namespace rocket
