#include <iostream>
#include <pthread.h>
#include "../rocket/common/log.h"
#include "../rocket/common/config.h"
// #include <tinyxml/tinyxml.h>
void* fun(void*) {
    DEBUGLOG("debug this is thread in %s", "fun");
    INFOLOG("info this is thread in %s", "fun");
    return NULL;
}

int main() {

    rocket::Config::SetGlobalConfig("/data/ai/Cplusplus/simple_rpc/conf/rocket.xml");

    pthread_t thread;
    pthread_create(&thread, NULL, &fun, NULL);
    DEBUGLOG("debug test log %s", "11");
    INFOLOG("info test log %s", "11");
    return 0;
}

// #include <iostream>

// #define eprintf(...) fprintf(stderr, __VA_ARGS__)

// int main() {
//     eprintf ("%s:  %d: ", "232", 13);
//     return 0;
// }
