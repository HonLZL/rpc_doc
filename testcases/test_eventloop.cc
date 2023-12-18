#include <pthread.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <argp.h>
#include <string.h>


#include "../rocket/common/config.h"
#include "../rocket/common/log.h"
#include "../rocket/net/fdevent.h"
#include "../rocket/net/eventloop.h"

// #include <tinyxml/tinyxml.h>
void* fun(void*) {
    int i = 100;
    while (i--) {
        DEBUGLOG("debug this is thread in %s", "fun");
        INFOLOG("info this is thread in %s", "fun");
    }
    return NULL;
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::EventLoop* eventloop = new rocket::EventLoop();

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        ERRORLOG("listen = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    
    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    
    
    return 0;
}