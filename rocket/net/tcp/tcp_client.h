/*
TcpClient

connetc => write => read => connect
非阻塞 Connect
Connect: 链接对端(peer)机器
Write: 将 RPC 响应发送给客户端
Read: 读取客户端发来的数据没,组装为 RPC 请求

返回 0, 表示连接成功
返回 -1,但 errno = EINPROGRESS, 表示连接正在建立,此时可以添加到 epoll 中去监听其可写事件,
等待可写事件就绪后,调用 getsockopt 获取 fd 上的错误,错误为 0 代表连接建立成功.
其他 errno 直接报错

*/
#ifndef ROCKET_NET_TCP_TCP_CLIENT_H
#define ROCKET_NET_TCP_TCP_CLIENT_H

#include <memory>

#include "../coder/abstract_protocol.h"
#include "../eventloop.h"
#include "net_addr.h"
#include "tcp_connection.h"

namespace rocket {
class TcpClient {
   public:
    typedef std::shared_ptr<TcpClient> s_ptr;

    TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();

    // 异步进行connect,如果connect成功, done 会被执行
    void connect(std::function<void()> done);

    // 异步发送 Message, 字符串 或 RPC 协议,发送成功,会调用 done 函数,函数的入参就是 message 对象
    void writeMessage(AbstractProtocol::s_ptr msssage, std::function<void(AbstractProtocol::s_ptr)> done);

    void readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);

    void stop();

   private:
    NetAddr::s_ptr m_peer_addr;
    EventLoop* m_event_loop{nullptr};
    int m_fd{-1};
    FdEvent* m_fd_event{nullptr};

    TcpConnection::s_ptr m_connection;
};

}  // namespace rocket

#endif
