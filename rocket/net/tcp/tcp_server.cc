#include "tcp_server.h"
#include "../../common/log.h"
#include "../eventloop.h"
#include "tcp_connection.h"

namespace rocket {
TcpServer::TcpServer(NetAddr::s_ptr local_addr)
    : m_local_addr(local_addr) {
    init();
    INFOLOG("rocket TcpServer listen success on [%s]", m_local_addr->toString().c_str());
}

TcpServer::~TcpServer() {
    if (m_main_event_loop) {
        delete m_main_event_loop;
        m_main_event_loop = nullptr;
    }

    if (m_io_thread_group) {
        delete m_io_thread_group;
        m_io_thread_group = nullptr;
    }
}

void TcpServer::init() {
    // make_shared: 用于创建一个std::shared_ptr智能指针
    // 创建一个使用 m_local_addr 初始化的 TcpAcceptor 的对象
    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);
    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    m_io_thread_group = new IOThreadGroup(2);

    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    auto re = m_acceptor->accept();
    int client_fd = re.first;
    NetAddr::s_ptr peer_addr = re.second;

    m_client_count++;

    // 将 clientfd 添加到任意 IO 线程里面
    IOThread* io_thread = m_io_thread_group->getIOThread();
    
    TcpConnection::s_ptr connetion = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr);

    connetion->setState(Connected); // 要设置状态为 connected, 然后才能进行 read 和 write

    m_client.insert(connetion);

    INFOLOG("TcpServer successfully get client, fd=%d", client_fd);
}

void TcpServer::start() {
    m_io_thread_group->start();
    m_main_event_loop->loop();
}

}  // namespace rocket
