#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>

#include "../common/log.h"
#include "../common/mutex.h"
#include "../common/util.h"
#include "eventloop.h"

#define ADD_TO_EPOLL()                                                                        \
    auto it = m_listen_fds.find(event->getFd());                                              \
    int op = EPOLL_CTL_ADD;                                                                   \
    if (it != m_listen_fds.end()) {                                                           \
        op = EPOLL_CTL_MOD;                                                                   \
    }                                                                                         \
    epoll_event tmp = event->getEpollEvent();                                                 \
    /*epoll_ctl: 管理红黑树上的文件描述符:添加,修改,删除*/                  \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                                 \
    if (rt == -1) {                                                                           \
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    }                                                                                         \
    DEBUGLOG("add event sucessfully, fd[%d]", event->getFd());

#define DEL_TO_EPOLL()                                                                        \
    auto it = m_listen_fds.find(event->getFd());                                              \
    if (it == m_listen_fds.end()) {                                                           \
        return;                                                                               \
    }                                                                                         \
    int op = EPOLL_CTL_DEL;                                                                   \
    epoll_event tmp = event->getEpollEvent();                                                 \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                                 \
    if (rt == -1) {                                                                           \
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    }                                                                                         \
    DEBUGLOG("delete event successfully, fd[%d]", event->getFd());

namespace rocket {

static thread_local EventLoop* t_current_eventloop = nullptr;

static int g_epoll_max_timeout = 10000;
static int g_epoll_max_events = 10;

EventLoop::EventLoop() {
    if (t_current_eventloop != nullptr) {
        ERRORLOG("failed to create event loop, this thread has created event loop [%d]\n", errno);
        exit(0);
    }
    m_thread_id = getThreadId();

    // 创建 epoll 实例,通过一棵红黑书管理待检测集合, 返回文件描述符,是个整数
    m_epoll_fd = epoll_create(1);  // 参数随便随便设置>0

    if (m_epoll_fd == -1) {
        ERRORLOG("failed to create event loop, epoll_create error, error info [%d]\n", errno);
        exit(0);
    }
    initWakeUpFdEvent();
    initTimer();

    INFOLOG("successfully create event loop in thread [%d]! ", m_thread_id);

    t_current_eventloop = this;
}

// 正常来说是调用不到析构函数的,因为服务是死循环,对象不会析构
EventLoop::~EventLoop() {
    close(m_epoll_fd);
    if (m_wakeup_fd_event) {
        delete m_wakeup_fd_event;
    }
}

void EventLoop::initTimer() {
    m_timer = new Timer();
    addEpollEvent(m_timer);
}

void EventLoop::addTimerEvent(TimerEvent::s_ptr event) {
    m_timer->addTimerEvent(event);
}

void EventLoop::initWakeUpFdEvent() {
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
    if (m_wakeup_fd < 0) {
        ERRORLOG("failed to create event loop, eventfd create error error info [%d]", errno);
        exit(0);
    }

    INFOLOG("wakeup fd = %d", m_wakeup_fd);

    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]() {
        char buf[8];
        // 数据读完了
        while (read(m_wakeup_fd, buf, 8) == -1 && errno != EAGAIN) {
        }
        DEBUGLOG("read full bytes from wakeup fd [%d]", m_wakeup_fd);
    });

    addEpollEvent(m_wakeup_fd_event);
}

void EventLoop::loop() {
    while (!m_stop_flag) {
        ScopeMutex<Mutex> lock(m_mutex);  // 把任务从队列拿出来的过程,要加锁
        std::queue<std::function<void()>> tem_tasks;
        m_pending_tasks.swap(tem_tasks);
        lock.unlock();

        while (!tem_tasks.empty()) {
            std::function<void()> cb = tem_tasks.front();
            tem_tasks.pop();
            if (cb) {
                cb();
            }
        }

        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];

        // DEBUGLOG("now begin to epoll_wait = %d \n", 0);

        // 检测 epoll 树中是否有就绪的文件描述符
        int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);

        DEBUGLOG("now end epoll_wait, rt = %d", rt);

        if (rt < 0) {
            ERRORLOG("epoll_wait errot, errno = %d", errno);
        } else {
            for (int i = 0; i < rt; i++) {
                epoll_event trigger_event = result_events[i];
                // static_cast 将隐式转换显式化表示出来 epoll_event => FdEvent
                FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);

                if (fd_event == nullptr) {
                    continue;
                }

                if (trigger_event.events & EPOLLIN) {
                    DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::IN_EVENT));
                }
                if (trigger_event.events & EPOLLOUT) {
                    DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::OUT_EVENT));
                }
            }
        }
    }
}

void EventLoop::wakeup() {
    INFOLOG("WAKE UP");
    m_wakeup_fd_event->wakeup();
}

void EventLoop::stop() {
    m_stop_flag = true;
    wakeup();
}

void EventLoop::dealWakeup() {
}

void EventLoop::addEpollEvent(FdEvent* event) {
    if (isInLoopThread()) {  // 当前函数的 io 线程
        ADD_TO_EPOLL();
    } else {  // 不是当前线程, 用回调函数
        auto cb = [this, event]() {
            ADD_TO_EPOLL();
        };
        addTask(cb, true);
    }
}
void EventLoop::deleteEpollEvent(FdEvent* event) {
    if (isInLoopThread()) {
        DEL_TO_EPOLL();
    } else {
        auto cb = [this, event]() {
            DEL_TO_EPOLL();
        };
        addTask(cb, true);
    }
}
void EventLoop::addTask(std::function<void()> cb, bool is_wakeup /*=false*/) {
    ScopeMutex<Mutex> lock(m_mutex);  // 向任务队列里加任务时,要加锁
    m_pending_tasks.push(cb);
    lock.unlock();
    if (is_wakeup) {
        wakeup();
    }
}

// 判断当前线程是不是 in loop IO 线程
bool EventLoop::isInLoopThread() {
    return getThreadId() == m_thread_id;
}

EventLoop* EventLoop::getCurrentEventLoop() {
    if(t_current_eventloop) {
        return t_current_eventloop;
    }
    t_current_eventloop = new EventLoop();
    return t_current_eventloop;
}

}  // namespace rocket
