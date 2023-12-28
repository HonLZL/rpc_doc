#ifndef ROCKET_NET_RPC_RPC_DISPATCHER_H
#define ROCKET_NET_RPC_RPC_DISPATCHER_H

#include <map>
#include <memory>
#include <google/protobuf/service.h>

#include "../coder/abstract_coder.h"
#include "../coder/abstract_protocol.h"
#include "../coder/tinypb_protocol.h"
// #include "../tcp/tcp_connection.h"

namespace rocket {
// 如果头文件引入的话,会导致循环依赖,所以采取前置定义的方法
class TcpConnection; 
class RpcDispatcher {
   public:
    static RpcDispatcher* GetRpcDispatcher();
    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;

    void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response,  TcpConnection* connection);

    void registerService(service_s_ptr service);

    void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info);

   private:
    bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);

   private:
    std::map<std::string, service_s_ptr> m_service_map;
};
}  // namespace rocket

#endif
