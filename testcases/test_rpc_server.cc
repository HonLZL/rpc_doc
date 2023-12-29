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

#include "../rocket/common/log.h"
#include "../rocket/net/coder/abstract_protocol.h"
#include "../rocket/net/coder/string_coder.h"
#include "../rocket/net/coder/tinypb_coder.h"
#include "../rocket/net/tcp/net_addr.h"
#include "../rocket/net/tcp/tcp_client.h"
#include "../rocket/net/tcp/tcp_server.h"
#include "../protobuf/order.pb.h"

class OrderImpl : public Order {
   public:
    virtual void makeOrder(google::protobuf::RpcController* controller,
                           const ::makeOrderRequest* request,
                           ::makeOrderResponse* response,
                           ::google::protobuf::Closure* done) {

        if(request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info(("low balance"));
            return;
        }
        response->set_order_id("20231228");
    }
};

void test_rpc_server() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12346);
    DEBUGLOG("create addr %s", addr->toString().c_str());
    rocket::TcpServer tcp_server(addr);

    tcp_server.start();
}


int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");

    rocket::Logger::InitGlobalLogger();

    // test_connect();

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    test_rpc_server();

    return 0;
}