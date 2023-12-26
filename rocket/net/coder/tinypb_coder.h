#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H

#include "abstract_coder.h"

namespace rocket {
class TinyPBCoder : public AbstractCoder {
   public:
    // 将 message 对象转化为 字节流,写入到 buffer
    void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) = 0;

    // 将buffer 里面的字节流转换为 message 对象
    void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) = 0;
};
}  // namespace rocket

#endif
