#include <memory>
#include <iostream>

#include "../rocket/common/log.h"
#include "../rocket/net/tcp/net_addr.h"
#include "../rocket/net/tcp/tcp_server.h"


void test_tcp_server() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12347);
    DEBUGLOG("create addr %s", addr->toString().c_str());
    rocket::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {
    rocket::Config::SetGlobalConfig("/data/ai/Cplusplus/simple_rpc/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_tcp_server();
    return 0;
}

