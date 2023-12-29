#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>

#include "../../common/error_code.h"
#include "../../common/log.h"
#include "../../common/msg_util.h"
#include "../net/coder/tinypb_protocol.h"
#include "../net/rpc/rpc_controller.h"
#include "../net/tcp/tcp_client.h"
#include "rpc_channel.h"

// namespace rocket {
// RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr)
//     : m_peer_addr(peer_addr) {
// }

// RpcChannel::~RpcChannel() {
//     INFOLOG("~RpcChannel");
// }

// void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
//                             google::protobuf::RpcController* controller,
//                             const google::protobuf::Message* request,
//                             google::protobuf::Message* response,
//                             google::protobuf::Closure* done) {
//     std::shared_ptr<TinyPBProtocol> req_protocol = std::make_shared<TinyPBProtocol>();
//     RpcController* m_controller = dynamic_cast<RpcController*>(controller);
//     if (m_controller == nullptr) {
//         ERRORLOG("failed callmethod, RpcController convert error");
//         return;
//     }
//     if (m_controller->GetMsgId().empty()) {
//         req_protocol->m_msg_id = MsgIdUtil::GenMsgId();
//         m_controller->SetMsgId(req_protocol->m_msg_id);
//     } else {
//         req_protocol->m_msg_id = m_controller->GetMsgId();
//     }
//     req_protocol->m_method_name = method->full_name();
//     INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

//     // 判断是否有被初始化
//     if (!m_is_init) {
//         ERRORLOG("rpc channel is not inited");
//         std::string err_info = "Rpc channel is not inited";
//         m_controller->SetError(ERROR_RPC_CHANNEL_INIT, err_info);
//         ERRORLOG("%s | %s,  origin request [%s]", req_protocol->m_msg_id.c_str(), err_info.c_str(),
//                  request->ShortDebugString().c_str());
//         return;
//     }

//     // request 序列化
//     if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
//         std::string err_info = "failed to serilize";
//         m_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
//         ERRORLOG("%s | %s,  origin request [%s]", req_protocol->m_msg_id.c_str(), err_info.c_str(),
//                  request->ShortDebugString().c_str());
//         return;
//     }

//     // 获得当前对象的智能指针
//     s_ptr channel = shared_from_this();

//     // connect  =>  write  =>  read
//     // 连接,发送协议,读回包最后执行回调函数 done
//     // client 使用引用传递,而不是值传递,值传递会进行复制,从而出错
//     // mutable 关键字:允许在 const 成员函数中修改被声明为 mutable 的成员变量的值。
//     m_client->connect([req_protocol, channel]() mutable {
//         channel->getTcpClient()->writeMessage(req_protocol, [req_protocol, channel](AbstractProtocol::s_ptr) mutable {
//             INFOLOG("%s | , send request success, call method name [%s]",
//                     req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

//             channel->getTcpClient()->readMessage(req_protocol->m_msg_id, [channel](AbstractProtocol::s_ptr msg) mutable {
//                 std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(msg);
//                 INFOLOG("%s | , send rpc response success, call method name [%s]",
//                         rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str());

//                 RpcController* m_controller = dynamic_cast<RpcController*>(channel->getController());
//                 if (!(channel->getResponse()->ParseFromString(rsp_protocol->m_pb_data))) {
//                     ERRORLOG("%s | serialize error", rsp_protocol->m_msg_id.c_str());
//                     m_controller->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
//                     return;
//                 }
                
//                 // 调用成功
//                 if (rsp_protocol->m_err_code != 0) {
//                     ERRORLOG("%s | call rpc methood[%s] failed, error code[%d], error info[%s]",
//                              rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
//                              rsp_protocol->m_err_code, rsp_protocol->m_err_info.c_str());

//                     m_controller->SetError(rsp_protocol->m_err_code, rsp_protocol->m_err_info);
//                     return;
//                 }

//                 if (channel->getClosure()) {
//                     channel->getClosure()->Run();
//                 }
//                 // 对于 shared_ptr channel 重置智能指针,置为空;其他指向同一对象的智能指针计数器减一
//                 channel.reset();
//             });
//         });
//     });
// }
// void RpcChannel::Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done) {
//     if (m_is_init) {
//         return;
//     }
//     m_controller = controller;
//     m_request = req;
//     m_response = res;
//     m_closure = done;
//     m_is_init = true;
// }
// google::protobuf::RpcController* RpcChannel::getController() {
//     // 用于获取指向被只能指针管理的对象的原始指针
//     return m_controller.get();
// }
// google::protobuf::Message* RpcChannel::getRequest() {
//     return m_request.get();
// }
// google::protobuf::Message* RpcChannel::getResponse() {
//     return m_response.get();
// }
// google::protobuf::Closure* RpcChannel::getClosure() {
//     return m_closure.get();
// }
// TcpClient*  RpcChannel::getTcpClient() {
//     return m_client.get();
// }
// }  // namespace rocket


namespace rocket {

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
  m_client = std::make_shared<TcpClient>(m_peer_addr);
}

RpcChannel::~RpcChannel() {
  INFOLOG("~RpcChannel");
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                        google::protobuf::Message* response, google::protobuf::Closure* done) {

  
  std::shared_ptr<rocket::TinyPBProtocol> req_protocol = std::make_shared<rocket::TinyPBProtocol>();

  RpcController* my_controller = dynamic_cast<RpcController*>(controller);
  if (my_controller == NULL) {
    ERRORLOG("failed callmethod, RpcController convert error");
    return;
  }

  if (my_controller->GetMsgId().empty()) {
    req_protocol->m_msg_id = MsgIdUtil::GenMsgId();
    my_controller->SetMsgId(req_protocol->m_msg_id);
  } else {
    req_protocol->m_msg_id = my_controller->GetMsgId();
  }

  req_protocol->m_method_name = method->full_name();
  INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

  if (!m_is_init) {

    std::string err_info = "RpcChannel not init";
    my_controller->SetError(ERROR_RPC_CHANNEL_INIT, err_info);
    ERRORLOG("%s | %s, RpcChannel not init ", req_protocol->m_msg_id.c_str(), err_info.c_str());
    return;
  }

  // requeset 的序列化
  if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
    std::string err_info = "failde to serialize";
    my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
    ERRORLOG("%s | %s, origin requeset [%s] ", req_protocol->m_msg_id.c_str(), err_info.c_str(), request->ShortDebugString().c_str());
    return;
  }

  s_ptr channel = shared_from_this(); 

  m_client->connect([req_protocol, channel]() mutable {
    channel->getTcpClient()->writeMessage(req_protocol, [req_protocol, channel](AbstractProtocol::s_ptr) mutable {
      INFOLOG("%s | send rpc request success. call method name[%s]", 
        req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

      channel->getTcpClient()->readMessage(req_protocol->m_msg_id, [channel](AbstractProtocol::s_ptr msg) mutable {
        std::shared_ptr<rocket::TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(msg);
        INFOLOG("%s | success get rpc response, call method name[%s]", 
          rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str());

        RpcController* my_controller = dynamic_cast<RpcController*>(channel->getController());
        if (!(channel->getResponse()->ParseFromString(rsp_protocol->m_pb_data))){
          ERRORLOG("%s | serialize error", rsp_protocol->m_msg_id.c_str());
          my_controller->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
          return;
        }

        if (rsp_protocol->m_err_code != 0) {
          ERRORLOG("%s | call rpc methood[%s] failed, error code[%d], error info[%s]", 
            rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
            rsp_protocol->m_err_code, rsp_protocol->m_err_info.c_str());

          my_controller->SetError(rsp_protocol->m_err_code, rsp_protocol->m_err_info);
          return;
        }

        if (channel->getClosure()) {
          channel->getClosure()->Run();
        }

        channel.reset();
      });
    });

  });

}


void RpcChannel::Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done) {
  if (m_is_init) {
    return;
  }
  m_controller = controller;
  m_request = req; 
  m_response = res;
  m_closure = done;
  m_is_init = true;
}

google::protobuf::RpcController* RpcChannel::getController() {
  return m_controller.get();
}

google::protobuf::Message* RpcChannel::getRequest() {
  return m_request.get();
}

google::protobuf::Message* RpcChannel::getResponse() {
  return m_response.get();
}

google::protobuf::Closure* RpcChannel::getClosure() {
  return m_closure.get();
}


TcpClient* RpcChannel::getTcpClient() {
  return m_client.get();
}

}