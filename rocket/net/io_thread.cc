#include <assert.h>
#include <pthread.h>

#include "../common/log.h"
#include "../common/util.h"
#include "io_thread.h"

namespace rocket {
IOThread::IOThread() {
    int rt = sem_init(&m_init_semaphore, 0, 0);  // 0 表示初始化成功
    assert(rt == 0);

    rt = sem_init(&m_start_semaphore, 0, 0);  // 0 表示初始化成功
    assert(rt == 0);

    pthread_create(&m_thread, nullptr, &IOThread::Main, this);

    // 一直等, 直到新线程执行完 Main 函数的前置,使用信号量机制
    sem_wait(&m_init_semaphore);
    DEBUGLOG("IOThread [%d] create successfully,", m_thread_id);
}

IOThread::~IOThread() {
    m_event_loop->stop();

    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    pthread_join(m_thread, nullptr);

    if (m_event_loop) {
        delete m_event_loop;

        // 在删除指针后，如果不将指针设置为 nullptr，它将继续指向先前分配的内存空间,成为悬空指针
        // 更清晰；避免重复删除;
        m_event_loop = nullptr;

        // 一个指针仍然保存着一个已经释放的内存地址，这个指针就被称为悬空指针, 使用悬空指针可能导致程序崩溃、数据损坏、内存泄漏等问题
        // 释放后未将指针置为nullptr; 超出作用域后未置nullptr; 指向使用已经删除的对象的指针
    }
}

// 使用 void* 类型的参数，要将其转换为其他类型的指针
void* IOThread::Main(void* arg) {
    IOThread* thread = static_cast<IOThread*>(arg);

    thread->m_event_loop = new EventLoop();
    thread->m_thread_id = getThreadId();

    // 等待gaixiancheg以上执行完毕
    sem_post(&thread->m_init_semaphore);

    // 让 IO 线程等待,直到我们主动启动, start()函数里的 sem_post 启动
    DEBUGLOG("IOThread %d created loop, waiting start semaphore  ", thread->m_thread_id);
    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("IOThread %d starts loop ", thread->m_thread_id);

    thread->m_event_loop->loop();
    DEBUGLOG("IOThread %d ends loop ", thread->m_thread_id);

    return nullptr;
}

EventLoop* IOThread::getEventLoop() {
    return m_event_loop;
}

void IOThread::start() {
    DEBUGLOG("Now invoke IOThread %d", m_thread_id);
    sem_post(&m_start_semaphore);
}

void IOThread::join() {
    pthread_join(m_thread, nullptr);
}

}  // namespace rocket