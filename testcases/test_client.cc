#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <string>

#include "../rocket/common/log.h"
#include "../rocket/net/tcp/net_addr.h"
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

    std::string msg = "hello worlds!";

    rt = write(fd, msg.c_str(), msg.length());

    DEBUGLOG("fd %d success write %d bytes, [%s]", fd, rt, msg.c_str());

    char buf[100];
    rt = read(fd, buf, 100);

    DEBUGLOG("fd %d success read %d bytes, [%s]", fd, rt, std::string(buf).c_str());
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_connect();

    return 0;
}
