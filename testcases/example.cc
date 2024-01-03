#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <google/protobuf/service.h>
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
#include "../rocket/net/tcp/net_addr.h"
#include "../rocket/net/tcp/tcp_client.h"
#include "../rocket/net/tcp/tcp_server.h"

class OrderImpl : public Order {
   public:
    virtual void makeOrder(google::protobuf::RpcController* controller,
                           const ::makeOrderRequest* request,
                           ::makeOrderResponse* response,
                           ::google::protobuf::Closure* done) {
        APPDEBUGLOG("start sleep 5s");
        sleep(5);
        APPDEBUGLOG("end sleep 5s");

        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info(("low balance"));
            return;
        }
        response->set_order_id("20231228");
        APPDEBUGLOG("call makeOrder success");
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("start error, argc != 2\n");
        printf("please input:  ./test_rpc_server ../conf/rocket.xml\n");
    }

    rocket::Config::SetGlobalConfig(argv[1]);

    rocket::Logger::InitGlobalLogger();

    // test_connect();

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>(
        "127.0.0.1", rocket::Config::GetGlobalConfig()->m_port);

    rocket::TcpServer tcp_server(addr);

    tcp_server.start();

    return 0;
}


