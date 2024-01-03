#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <string>

#include "../protobuf/order.pb.h"
#include "../rocket/common/log.h"
#include "../rocket/net/coder/abstract_protocol.h"
#include "../rocket/net/coder/string_coder.h"
#include "../rocket/net/coder/tinypb_coder.h"
#include "../rocket/net/rpc/rpc_channel.h"
#include "../rocket/net/rpc/rpc_closure.h"
#include "../rocket/net/rpc/rpc_controller.h"
#include "../rocket/net/tcp/net_addr.h"
#include "../rocket/net/tcp/tcp_client.h"
#include "../rocket/net/tcp/tcp_server.h"

void test_tcp_client() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12347);
    rocket::TcpClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("conenct to [%s] success", addr->toString().c_str());
        std::shared_ptr<rocket::TinyPBProtocol> message = std::make_shared<rocket::TinyPBProtocol>();
        message->info = "hello rocket!";
        message->m_msg_id = "123456789";
        message->m_pb_data = "test pb data";

        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");
        int rt = request.SerializeToString(&(message->m_pb_data));
        if (!rt) {
            DEBUGLOG("serialize error");
            return;
        }

        message->m_method_name = "Order.makeOrder";

        client.writeMessage(message, [request](rocket::AbstractProtocol::s_ptr msg_ptr) {
            DEBUGLOG("send message success, request [%s]", request.ShortDebugString().c_str());
        });

        client.readMessage("123456789", [](rocket::AbstractProtocol::s_ptr msg_ptr) {
            // 智能指针转换工具，用于在继承关系中安全地将一个智能指针转换为另一个相关类型的智能指针
            // 此处是 基类:AbstractProtocol 转化为 派生类:StringProtocol
            std::shared_ptr<rocket::TinyPBProtocol> message = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("msg_id[%s], get response [%s]", message->m_msg_id.c_str(), message->m_pb_data.c_str());

            makeOrderResponse response;
            int rt = response.ParseFromString(message->m_pb_data);
            if (!rt) {
                ERRORLOG("deserialize error");
                return;
            }
            DEBUGLOG("get response success, response [%s]", response.ShortDebugString().c_str())
        });

        client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr) {
            DEBUGLOG("send message 22222 success");
        });
    });
}

void test_rpc_channel() {
    // rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12346);
    // std::shared_ptr<rocket::RpcChannel> channel = std::make_shared<rocket::RpcChannel>(addr);

    NEWRPCCHANNEL("127.0.0.1:12346", channel);

    // std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();

    // 在编译之前,宏在预处理时进行替换,所以在这不需要加 rocket::
    NEWMESSAGE(makeOrderRequest, request);
    request->set_price(100);
    request->set_goods("apple");

    // std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();
    NEWMESSAGE(makeOrderResponse, response);

    // std::shared_ptr<rocket::RpcController> controller = std::make_shared<rocket::RpcController>();
    NEWRPCCONTROLLER(controller);
    controller->SetMsgId("999888");
    controller->SetTimeout(10000); // ms

    // 构造回调函数
    std::shared_ptr<rocket::RpcClosure> closure = std::make_shared<rocket::RpcClosure>([controller, request, response, channel]() mutable {
        if (controller->GetErrorCode() == 0) {
            INFOLOG("call rpc success, request [%s], response [%s]",
                    request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
            // 执行其他业务逻辑
        } else {
            INFOLOG("call rpc failedd, request [%s], error code [%d], error info [%s]",
                    request->ShortDebugString().c_str(), controller->GetErrorCode(), controller->GetErrorInfo().c_str());
        }
        INFOLOG("now exit eventloop");
        // channel->getTcpClient()->stop();
        channel.reset();
    });
    CALLRPC("127.0.0.1:12346", makeOrder, controller, request, response, closure);
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket_client.xml");

    rocket::Logger::InitGlobalLogger();

    // test_connect();

    test_rpc_channel();

    return 0;
}