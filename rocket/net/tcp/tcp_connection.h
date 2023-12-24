/*
TcpConnection:
read => excute => write => read ...
read: 读取客户端发来的数据,组装为 RPC 请求
excute: 将 RPC 请求作为传入参数,执行业务逻辑得到 RPC 请求
write: 将 RPC 响应发送给客户端

*/

#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H

#include "../io_thread.h"
#include "net_addr.h"
#include "tcp_buffer.h"

namespace rocket {

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4,
};
class TcpConnection {
   public:
    typedef std::shared_ptr<TcpConnection> s_ptr;

    // 属于哪个 IO 线程; 表示哪个客户端; 初始化 buffer 大小; 对端地址
    TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
    ~TcpConnection();
    void onRead();
    void excute();
    void onWrite();

    void setState(const TcpState state);

    TcpState getState();

    void clear();

    // 服务器主动连接,主动关闭一些恶意或者无效的连接,触发四次挥手
    void shutdown();

   private:

    IOThread* m_io_thread {nullptr};  // 代表持有该连接的 IO 线程
    
    int m_fd{0};
    
    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer;   // 接收缓冲区
    TcpBuffer::s_ptr m_out_buffer;  // 发送缓冲区

    // EventLoop* m_event_loop {nullptr};  // 代表持有该连接的 IO 线程

    FdEvent* m_fd_event{nullptr};


    TcpState m_state;

};
}  // namespace rocket


#endif