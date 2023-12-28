#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <string>

#include "../../common/error_code.h"
#include "../../common/log.h"
#include "../coder/tinypb_protocol.h"
#include "../tcp/net_addr.h"
#include "../tcp/tcp_connection.h"
#include "rpc_controller.h"
#include "rpc_dispatcher.h"

#define DELETE_RESOURCE(XX) \
    if (XX != NULL) {       \
        delete XX;          \
        XX = NULL;          \
    }

namespace rocket {

static RpcDispatcher* g_rpc_dispatcher = nullptr;
RpcDispatcher* RpcDispatcher::GetRpcDispatcher() {
    if (g_rpc_dispatcher != nullptr) {
        return g_rpc_dispatcher;
    }
    g_rpc_dispatcher = new RpcDispatcher();
    return g_rpc_dispatcher;
}

void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection) {
    // 1 强制类型转换,AbstractProtocol => TinyPBProtocol
    // 2 获得方法名,解析出,服务名和方法名
    std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);

    std::string method_full_name = req_protocol->m_method_name;
    std::string service_name;
    std::string method_name;

    rsp_protocol->m_req_id = req_protocol->m_req_id;
    rsp_protocol->m_method_name = req_protocol->m_method_name;

    bool rt = parseServiceFullName(method_full_name, service_name, method_name);
    if (!rt) {
        ERRORLOG("rep_id [%s] | parse service name error [%d]", req_protocol->m_req_id.c_str(), rt);
        setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
        return;
    }

    auto it = m_service_map.find(service_name);
    if (it == m_service_map.end()) {
        ERRORLOG("req_id [%s] | service name[%s] not found", req_protocol->m_req_id.c_str(), service_name.c_str());
        setTinyPBError(req_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
        return;
    }

    service_s_ptr service = (*it).second;

    // 通过方法名获得 method 对象
    const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
    if (method == nullptr) {
        ERRORLOG("req_id [%s] | method name[%s] not found in service [%s]",
                 req_protocol->m_req_id.c_str(), service_name.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_METHOD_NOT_FOUND, "method name not found");
        return;
    }

    google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();

    // 反序列化,将 pb_data 反序列化为 req_msg
    rt = req_msg->ParseFromString(req_protocol->m_pb_data);
    if (!rt) {
        // 出错, 后面补充出错处理
        ERRORLOG("req_id [%s] | deserilize error", req_protocol->m_req_id.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
        DELETE_RESOURCE(req_msg);
        return;
    }

    INFOLOG("req_id = [%s], get rpc request = [%s]", req_protocol->m_req_id.c_str(), req_msg->ShortDebugString().c_str());

    google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

    // Controller
    RpcController rpc_controller;
    IPNetAddr::s_ptr local_addr = std::make_shared<IPNetAddr>("127.0.0.1", 12347);
    rpc_controller.SetLocalAddr(connection->getLocalAddr());
    rpc_controller.SetPeerAddr(connection->getPeerAddr());
    rpc_controller.SetReqId(req_protocol->m_req_id);

    service->CallMethod(method, &rpc_controller, req_msg, rsp_msg, nullptr);

    // 序列化, 将序列化的结果存入 m_pb_data
    rt = rsp_msg->SerializeToString(&rsp_protocol->m_pb_data);
    if (!rt) {
        ERRORLOG("req_id = [%s] | serilize error, origin message [%s]",
                 req_protocol->m_req_id.c_str(), rsp_msg->ShortDebugString().c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_SERIALIZE, "serilize error");
        return;
    }

    rsp_protocol->m_err_code = 0;
    INFOLOG("req_id = [%s] | dispatch success, request[%s], reponse[%s]",
            req_protocol->m_req_id.c_str(), req_msg->ShortDebugString().c_str(), rsp_msg->ShortDebugString().c_str());
}

void RpcDispatcher::registerService(service_s_ptr service) {
    std::string service_name = service->GetDescriptor()->full_name();
    m_service_map[service_name] = service;
}

bool RpcDispatcher::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
    if (full_name.empty()) {
        ERRORLOG("full_name is empty");
        return false;
    }
    size_t i = full_name.find_first_of(".");
    if (i == full_name.npos) {
        ERRORLOG("cant find . in full name [%s]", full_name);
        return false;
    }
    service_name = full_name.substr(0, i);
    method_name = full_name.substr(i + 1, full_name.length() - i - 1);

    INFOLOG("parse service_name[%s] and method_name[%s] from full name [%s]",
            service_name.c_str(), method_name.c_str(), full_name.c_str());
    return true;
}

void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info) {
    msg->m_err_code = err_code;
    msg->m_err_info = err_info;
    msg->m_err_info_len = err_info.length();
}

}  // namespace rocket
