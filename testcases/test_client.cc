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

#include "../rocket/common/log.h"
#include "../rocket/net/tcp/net_addr.h"
#include "../rocket/net/tcp/tcp_client.h"
#include "../rocket/net/tcp/tcp_server.h"

void test_connect() {
    // 调用 connect 连接 server
    // write 发送一个字符串
    // 等待 read 返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12347);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    // reinterpret_cast, 强制转换,
    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    if (rt == -1) {
        ERRORLOG("connect failed %d", fd);
        exit(0);
    }

    DEBUGLOG("connect successfully");

    std::string msg = "hello world!";

    rt = write(fd, msg.c_str(), msg.length());

    DEBUGLOG("fd %d success write %d bytes, [%s]", fd, rt, msg.c_str());

    char buf[100];
    rt = read(fd, buf, 100);

    if (rt >= 0) {
        buf[rt] = '\0';  // 添加字符串终止符号
        DEBUGLOG("fd %d success read %d bytes, [%s]", fd, rt, std::string(buf).c_str());
    }

    
}

void test_tcp_client() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12347);
    rocket::TcpClient client(addr);
    client.connect([addr]() {
        DEBUGLOG("connect to [%s] successfully", addr->toString().c_str());
    });
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    // test_connect();

    test_tcp_client();

    return 0;
}
