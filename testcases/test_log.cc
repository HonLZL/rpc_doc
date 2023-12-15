#include <iostream>
#include <pthread.h>
#include "../rocket/common/log.h"

void* fun(void*) {
    DEBUGLOG("this is thread in %s", "fun");
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, &fun, NULL);
    DEBUGLOG("test log %s", "11");
    return 0;
}
