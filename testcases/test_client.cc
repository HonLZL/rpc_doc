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
#include "../rocket/net/coder/abstract_protocol.h"
#include "../rocket/net/coder/string_coder.h"
#include "../rocket/net/coder/tinypb_coder.h"
#include "../rocket/net/tcp/net_addr.h"
#include "../rocket/net/tcp/tcp_client.h"
#include "../rocket/net/tcp/tcp_server.h"



void test_connect() {

  // 调用 conenct 连接 server
  // wirte 一个字符串
  // 等待 read 返回结果

  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    ERRORLOG("invalid fd %d", fd);
    exit(0);
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(12347);
  inet_aton("127.0.0.1", &server_addr.sin_addr);

  int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

  DEBUGLOG("connect success");

  std::string msg = "hello rocket!";
  
  rt = write(fd, msg.c_str(), msg.length());

  DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

  char buf[100];
  rt = read(fd, buf, 100);
  DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());

}

void test_tcp_client() {

  rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12347);
  rocket::TcpClient client(addr);
  client.connect([addr, &client]() {
    DEBUGLOG("conenct to [%s] success", addr->toString().c_str());
    std::shared_ptr<rocket::TinyPBProtocol> message = std::make_shared<rocket::TinyPBProtocol>();
    message->info = "hello rocket!";
    message->m_req_id = "123456789";
    message->m_pb_data = "test pb data";
    client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr) {
      DEBUGLOG("send message success");
    });

    client.readMessage("123456789", [](rocket::AbstractProtocol::s_ptr msg_ptr) {
      // 智能指针转换工具，用于在继承关系中安全地将一个智能指针转换为另一个相关类型的智能指针
      // 此处是 基类:AbstractProtocol 转化为 派生类:StringProtocol
      std::shared_ptr<rocket::TinyPBProtocol> message = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(msg_ptr);
      DEBUGLOG("req_id[%s], get response [%s]", message->m_req_id.c_str(), message->m_pb_data.c_str());
    });

    client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr) {
      DEBUGLOG("send message 22222 success");
    });
  });
}

int main() {

  rocket::Config::SetGlobalConfig("../conf/rocket.xml");

  rocket::Logger::InitGlobalLogger();

  // test_connect();

  test_tcp_client();

  return 0;
}