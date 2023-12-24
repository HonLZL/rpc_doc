#include <unistd.h>
#include <iostream>
#include "../../common/log.h"
#include "tcp_connection.h"
#include "fd_event_group.h"

namespace rocket {
TcpConnection::TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr)
    : m_io_thread(io_thread), m_fd(fd), m_peer_addr(peer_addr), m_state(NotConnected) {

    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
  io_thread->getEventLoop()->addEpollEvent(m_fd_event);
}
TcpConnection::~TcpConnection() {
    DEBUGLOG("~TcpConnection");
}
void TcpConnection::onRead() {
    // 1 从 socket 缓冲区,调用系统的 read 函数读取字节 in_buffer 里面
    if (m_state != Connected) {
        ERRORLOG("onRead error, client has already disconnected, addr[%s], clientfd [%d]",
                 m_peer_addr->toString().c_str(), m_fd);
        return;
    }
    bool is_read_all = false;
    bool is_close = false;

    while (!is_read_all) {
        if (m_in_buffer->writeAble() == 0) {  // 扩容
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }

        int read_count = m_in_buffer->writeAble();  // 获取能够写入的字节数
        int write_index = m_in_buffer->writeIndex();

        int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);

        DEBUGLOG("successfully read %d bytes feom addr [%s], client fd [%d]",
                 rt, m_peer_addr->toString().c_str(), m_fd);
                 
        if (rt > 0) {
            m_in_buffer->moveWriteIndex(rt);
            if (rt == read_count) {  // 有可能没读完,要继续读
                continue;
            } else if (rt < read_count) {
                is_read_all = true;
                break;
            }            

        } else if (rt == 0) {
            is_close = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {  // 读完了,没有数据了
            is_read_all = true;
            break;
        }
    }

    if (is_close) {
        INFOLOG("peer closed, peer addr [%s], cliend [%d]", m_peer_addr->toString().c_str(), m_fd);
        clear();
        return;
    }

    if (!is_read_all) {
        DEBUGLOG("not read all data");
    }
    // TODO: 简单的 echo,后面补充 RPC 协议解析
    excute();
}
void TcpConnection::excute() {

    // 将 RPC 请求执行业务逻辑,获取 RPC 响应, 再把 RPC 响应发送出去
    std::vector<char> tmp;
    int size = m_in_buffer->readAble();
    tmp.resize(size);

    m_in_buffer->readFromBuffer(tmp, size);

    std::string msg;
    for (size_t i = 0; i < tmp.size(); i++) {
        msg += tmp[i];
    }

    INFOLOG("success get request from client [%s], str = %s", msg.c_str(), m_peer_addr->toString().c_str());

    m_out_buffer->writeToBuffer(msg.c_str(), msg.length());

    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    

    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);  // 添加到 eventloop 内
}
void TcpConnection::onWrite() {
    // 将当前 out_buffer 里面的数据全部发送非 client, 循环调用 write 函数发送
    if (m_state != Connected) {
        ERRORLOG("onWrite error, client has already disconnected, addr[%s], clientfd [%d]",
                 m_peer_addr->toString().c_str(), m_fd);
        return;
    }
    bool is_write_all = false;
    while (true) {
        if (m_out_buffer->readAble() == 0) {  // 没有数据发送了
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();

        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);
        if (rt >= write_size) {  // 判断是否发送完
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            // 发送缓冲区已满,不能再发送了,需要等待下次 fd 可写的时候再次发送数据
            ERRORLOG("write data error, errno==EAGIN and re == -1");  
            break;
        }
    }
    if(is_write_all) {
        m_fd_event->cancleListen(FdEvent::OUT_EVENT);
        m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);  // 修改,并不是删除,只是取消监听
    }
}
void TcpConnection::setState(const TcpState state) {
    m_state = state;
}

TcpState TcpConnection::getState() {
    return m_state;
}

void TcpConnection::clear() {
    // 处理一些关闭连接后的清理动作
    if (m_state == Closed) {
        return;
    }
    m_fd_event->cancleListen(FdEvent::IN_EVENT);
    m_fd_event->cancleListen(FdEvent::OUT_EVENT);
    
    m_io_thread->getEventLoop()->deleteEpollEvent(m_fd_event);
    m_state = Closed;
}

void TcpConnection::shutdown() {
    if(m_state == Closed || m_state == NotConnected) {
        return;
    }
    // 处于半关闭状态
    m_state = HalfClosing;

    // 调用系统函数 shutdown,关闭读写,意味着服务器不再对这个 fd 进行读写操作了
    // 发送 FIN 报文,触发了四次挥手的第一个阶段
    // 当 fd 发生可读事件,但是可读的数据为 0 ,即对端发送了 FIN
    ::shutdown(m_fd, SHUT_RDWR);

}

}  // namespace rocket