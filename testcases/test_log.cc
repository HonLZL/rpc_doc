#include <pthread.h>
#include <iostream>
#include "../rocket/common/config.h"
#include "../rocket/common/log.h"
// #include <tinyxml/tinyxml.h>
void* fun(void*) {
    int i = 1;
    while (i--) {
        DEBUGLOG("debug this is thread in %s", "fun");
        INFOLOG("info this is thread in %s", "fun");
    }
    return nullptr;
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    std::cout << "873t2bfg" << std::endl;

    DEBUGLOG("debug test log %s", "11");
    // pthread_t thread;
    // pthread_create(&thread, nullptr, &fun, nullptr);
    // int i = 1;
    // while (i--) {
    //     DEBUGLOG("debug test log %s", "11");
    //     INFOLOG("info test log %s", "11");
    // }

    // pthread_join(thread, nullptr);
    return 0;
}

// #include <iostream>

// #define eprintf(...) fprintf(stderr, __VA_ARGS__)

// int main() {
//     eprintf ("%s:  %d: ", "232", 13);
//     return 0;
// }
