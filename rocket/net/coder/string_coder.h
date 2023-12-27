#ifndef ROCKET_NET_STRING_CODER_H
#define ROCKET_NET_STRING_CODER_H

#include <string>
#include "abstract_protocol.h"
#include "abstract_coder.h"

namespace rocket {

class StringProtocol : public AbstractProtocol {
    public:
     std::string info;
};

class StringCoder : public AbstractCoder {
    // 将 message 对象转化为 字节流,写入到 buffer
    void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {
        std::string msg = "encode hello world!";
        out_buffer->writeToBuffer(msg.c_str(), msg.length());
        for(size_t i=0;i<messages.size();i++) {
            // StringProtocol* msg = dynamic_cast<StringProtocol*> (messages[i]);
            std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(messages[i]);
            out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
        }
    }
    // 将buffer 里面的字节流转换为 message 对象
    void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {
        std::vector<char> re;
        buffer->readFromBuffer(re, buffer->readAble());
        std::string info;
        for (size_t i = 0; i < re.size(); i++) {
            info += re[i];
        }
        std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
        msg->info = info;   
        msg->m_req_id = "123456789";
        out_messages.push_back(msg);
    }
};
}  // namespace rocket

#endif