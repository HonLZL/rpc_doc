#include <pthread.h>
#include <iostream>
#include "../rocket/common/config.h"
#include "../rocket/common/log.h"
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

    pthread_t thread;
    pthread_create(&thread, NULL, &fun, NULL);
    int i = 100;
    while (i--) {
        DEBUGLOG("debug test log %s", "11");
        INFOLOG("info test log %s", "11");
    }

    pthread_join(thread, NULL);
    return 0;
}

// #include <iostream>

// #define eprintf(...) fprintf(stderr, __VA_ARGS__)

// int main() {
//     eprintf ("%s:  %d: ", "232", 13);
//     return 0;
// }
