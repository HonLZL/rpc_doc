#ifndef ROCKET_NET_ABSTRACT_CODER_H
#define ROCKET_NET_ABSTRACT_CODER_H

#include <vector>

#include "abstract_protocol.h"
#include "../tcp/tcp_buffer.h"

namespace rocket {
class AbstractCoder {
   public:
    // 将 message 对象转化为 字节流,写入到 buffer
    virtual void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) = 0;

    // 将buffer 里面的字节流转换为 message 对象
    virtual void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) = 0;

    // 类有虚拟函数，特别是拥有虚拟函数的基类，要为其定义一个虚拟析构函数
    // 否则，在通过基类指针删除派生类对象时，只会调用基类的析构函数，而不会调用派生类的析构函数，
    // 可能导致资源泄漏或者未正确释放资源的情况发生。
    virtual ~AbstractCoder() {}
};

}  // namespace rocket

#endif