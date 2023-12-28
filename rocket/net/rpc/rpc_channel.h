/*
RpcChannel
客户端服务端通信
connect ->  encode  ->  write  ->   read  ->   decode
*/

#ifndef ROCKET_NET_RPC_RPC_CHANNEL_H
#define ROCKET_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>

#include "../tcp/net_addr.h"

namespace rocket {
class RpcChannel : public google::protobuf::RpcChannel {
   public:
    RpcChannel(NetAddr::s_ptr peer_addr);
    ~RpcChannel();

   public:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done);

   private:
    NetAddr::s_ptr m_peer_addr;
    NetAddr::s_ptr m_local_addr;
};

}  // namespace rocket

#endif