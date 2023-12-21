#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

#include "../rocket/common/config.h"
#include "../rocket/common/log.h"
#include "../rocket/net/eventloop.h"
#include "../rocket/net/fd_event.h"
#include "../rocket/net/io_thread.h"
#include "../rocket/net/timer_event.h"

void test_io_thread() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12310);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);

    int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rt != 0) {
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(listenfd, 100);
    if (rt != 0) {
        ERRORLOG("listen error");
        exit(1);
    }

    rocket::FdEvent event(listenfd);
    event.listen(rocket::FdEvent::IN_EVENT, [listenfd]() {
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

        inet_ntoa(peer_addr.sin_addr);
        DEBUGLOG("successfully get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    });

    int i = 0;
    rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(
        1000, true, [&i]() {
            INFOLOG("trigger timer event, count= %d", i++);
        });
    
    // 栈对象离开函数会析构, 在函数中创建的对象，在函数结束时会被销毁
    // 栈上创建的对象，也就是局部对象，当其所在的函数执行结束时会被自动销毁和析构。
    // 堆上创建的对象需要手动管理其生命周期, new 和 delete
    rocket::IOThread io_thread;

    // FdEvent 有可读事件发生,EventLoop 会执行读回调函数
    io_thread.getEventLoop()->addEpollEvent(&event);
    io_thread.getEventLoop()->addTimerEvent(timer_event);

    io_thread.start();
    io_thread.join();
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::EventLoop* eventloop = new rocket::EventLoop();

    test_io_thread();

    // int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (listenfd == -1) {
    //     ERRORLOG("listenfd = -1");
    //     exit(0);
    // }

    // sockaddr_in addr;
    // memset(&addr, 0, sizeof(addr));

    // addr.sin_port = htons(12310);
    // addr.sin_family = AF_INET;
    // inet_aton("127.0.0.1", &addr.sin_addr);

    // int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    // if (rt != 0) {
    //     ERRORLOG("bind error");
    //     exit(1);
    // }

    // rt = listen(listenfd, 100);
    // if (rt != 0) {
    //     ERRORLOG("listen error");
    //     exit(1);
    // }

    // 设置读回调函数,这里是监听,获取客户端连接,并打印出来
    // rocket::FdEvent event(listenfd);
    // event.listen(rocket::FdEvent::IN_EVENT, [listenfd]() {
    //     sockaddr_in peer_addr;
    //     socklen_t addr_len = sizeof(peer_addr);
    //     memset(&peer_addr, 0, sizeof(peer_addr));
    //     int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

    //     inet_ntoa(peer_addr.sin_addr);
    //     DEBUGLOG("successfully get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    // });
    // eventloop->addEpollEvent(&event);

    // 定时任务,每秒去打印一次 INFOLOG
    // int i=0;
    // rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent>(
    //     1000, true, [&i]() {
    //         INFOLOG("trigger timer event, count= %d",  i++);
    //     }
    // );
    // eventloop->addTimerEvent(timer_event);

    // eventloop->loop();

    return 0;
}
