/*
TcpAcceptor

socket 流程
socket ---> bind ---> listen ---> accept

*/

#ifndef ROCKET_NET_TCP_TCP_ACCEPTOR_H
#define ROCKET_NET_TCP_TCP_ACCEPTOR_H

#include "net_addr.h"

namespace rocket {
class TcpAcceptor {
   public:
    typedef std::shared_ptr<TcpAcceptor> s_ptr;

    TcpAcceptor(NetAddr::s_ptr local_addr);
    ~TcpAcceptor();

    int accept();

    int getListenFd();

   private:
    // 服务端监听的地址, addr -> ip:port
    NetAddr::s_ptr m_local_addr;
    int m_family {-1};

    int m_listenfd{-1};
};
}  // namespace rocket

#endif