#include <string.h>
#include <vector>

#include "../../common/log.h"
#include "../../common/util.h"
#include "tinypb_coder.h"
#include "tinypb_protocol.h"

namespace rocket {
// 将 message 对象转化为 字节流,写入到 buffer
void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {
}

// 将buffer 里面的字节流转换为 message 对象
void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {
    // 遍历 buffer,找到 PB_START(0x02),找到之后,解析出整包的长度,
    // 然后得到结束符的位置,判断是否为 PB_START(0x03)
    while(true) {
        std::vector<char> tmp = buffer->m_buffer;
        int start_index = buffer->readIndex();
        int end_index = -1;

        int pk_len = 0;
        int parse_success = false;
        int i = 0;
        for (i = start_index; i < buffer->writeIndex(); i++) {
            if (tmp[i] == TinyPBProtocol::PB_START) {
                // 读下去四个字节,由于是网络字节序,需要转为主机字节序
                if (i + 1 < buffer->writeIndex()) {
                    pk_len = getInt32FromNetByte(&tmp[i + 1]);
                    DEBUGLOG("get pk_len = %d", pk_len);

                    // 结束符的索引
                    int j = i + pk_len - 1;
                    if (j >= buffer->writeIndex()) {
                        continue;
                    }
                    if (tmp[j] == TinyPBProtocol::PB_END) {
                        start_index = i;
                        end_index = j;
                        parse_success = true;
                    }
                }
            }
        }
        
        if(i >= buffer->writeIndex()) {
            DEBUGLOG("decode end, read all buffer data");
            return;
        }

        if (parse_success) {
            buffer->moveReadIndex(end_index - start_index);
            // 指针指向类的实例
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            message->m_pk_len = pk_len;

            // req_id
            int req_id_len_index = start_index + sizeof(char) + sizeof(message->m_pk_len);
            if (req_id_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, req_id_len_index [%d] >= end_index[%d]", req_id_len_index, end_index);
            }
            message->m_req_id_len = getInt32FromNetByte(&tmp[req_id_len_index]);
            DEBUGLOG("parse req_id_len=%d", message->m_req_id_len);
            int req_id_index = req_id_len_index + sizeof(message->m_req_id_len);
            char req_id[100] = {0};
            memcpy(&req_id[0], &tmp[req_id_index], message->m_req_id_len);
            message->m_req_id = std::string(req_id);
            DEBUGLOG("parse req_id_len = %d", message->m_req_id.c_str());

            // method
            int method_name_len_index = req_id_index + message->m_req_id_len;
            if (method_name_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, req_id_len_index [%d] >= end_index[%d]", method_name_len_index, end_index);
            }
            message->m_method_name_len = getInt32FromNetByte(&tmp[method_name_len_index]);
            DEBUGLOG("parse m_method_name_len=%d", message->m_method_name_len);
            int method_name_index = method_name_len_index + sizeof(message->m_method_name_len);
            char method_name[512] = {0};
            memcpy(&method_name[0], &tmp[method_name_index], message->m_method_name_len);
            message->m_method_name = std::string(method_name);
            DEBUGLOG("parse m_method_name_len = %d", message->m_method_name.c_str());

            // error
            int err_info_len_index = method_name_index + message->m_method_name_len;
            if (err_info_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, err_info_len_index [%d] >= end_index[%d]", err_info_len_index, end_index);
            }
            message->m_err_info_len = getInt32FromNetByte(&tmp[err_info_len_index]);
            DEBUGLOG("parse m_err_info_len=%d", message->m_err_info_len);
            int err_info_index = err_info_len_index + sizeof(message->m_err_info_len);
            char error_info[512] = {0};
            memcpy(&error_info[0], &tmp[err_info_index], message->m_err_info_len);
            message->m_err_info = std::string(error_info);
            DEBUGLOG("parse m_err_info_len = %d", message->m_err_info.c_str());

            // pb_data
            int pb_data_len = message->m_pk_len - message->m_method_name_len -
                              message->m_req_id_len - message->m_err_info_len - 2 - 24;
            int pb_data_index = err_info_index + message->m_err_info_len;
            message->m_pb_data = std::string(&tmp[pb_data_index], pb_data_len);
            // 校验和解析
            message->parse_success = true;

            out_messages.push_back(message);
        }
    }
}

}  // namespace rocket
