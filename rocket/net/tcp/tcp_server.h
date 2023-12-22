/*
TcpServer 主从 Reactor
mainRector 由主线程运行,作用: 通过 epoll 监听 listenfd 的可读事件
当可读事件发生后,调用 accept 函数获取 clientfd,然后随机取出一个 subReactor,
将 client fd 的读写事件注册到这个 subRreactor 的 epoll 上即可,mainReactor
之负责 建立连接事件,不进行业务处理,也不关心已连接套接字的 IO 事件.
subReactor 通常有多个,每个 subReactor 由一个线程来运行,其注册 clientfd 的读写
事件,当发生 IO 事件后,需要进行业务处理.
*/

#ifndef ROCKET_COMMON_NET_TCP_TCP_SERVER_H
#define ROCKET_COMMON_NET_TCP_TCP_SERVER_H

#include "../eventloop.h"
#include "io_thread_group.h"
#include "net_addr.h"
#include "tcp_acceptor.h"

namespace rocket {
class TcpServer {
   public:
    TcpServer(NetAddr::s_ptr local_addr);
    ~TcpServer();

    void start();

   private:
    void init();

    // 当有新客户端连接之后,需要执行
    void onAccept();

   private:
    TcpAcceptor::s_ptr m_acceptor;
    NetAddr::s_ptr m_local_addr;  // 本地监听地址

    EventLoop* m_main_event_loop{nullptr};      // mainReactor
    IOThreadGroup* m_io_thread_group{nullptr};  // subReactor 组
    FdEvent* m_listen_fd_event;

    int m_client_count{0};
};
}  // namespace rocket

#endif
