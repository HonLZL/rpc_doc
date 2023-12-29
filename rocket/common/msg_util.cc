#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "msg_util.h"

namespace rocket {

static int g_msg_id_length = 20;  // 默认  ID 长度
static int g_random_fd = -1;      // 默认  ID 长度

static thread_local std::string t_msg_id_no;
static thread_local std::string t_max_msg_id_no;

std::string MsgIdUtil::GenMsgId() {
    // 当前 msg_id 是空的或者达到了最大id,要随机生成
    // 假设长度为5, t_msg_id_no 为长度为 5 的以数字组成的字符串, t_max_msg_id_no 是 "99999"
    if (t_msg_id_no.empty() || t_msg_id_no == t_max_msg_id_no) { 
        // /dev/urandom 是一个伪随机数生成器设备，它可以提供随机数据
        // 打开 linux 的随机文件, 获得随机文件的句柄
        if (g_random_fd == -1) {
            g_random_fd = open("/dev/urandom", O_RDONLY);
        }

        std::string res(g_msg_id_length, 0);
        if (read(g_random_fd, &res[0], g_msg_id_length) != g_msg_id_length) {
            ERRORLOG("read from /dev/urandom error");
            return "";
        }
        // 假设 g_msg_id_length = 5, 那么 t_max_msg_id_no = "99999"
        for (int i = 0; i < g_msg_id_length; i++) {
            uint8_t x = ((uint8_t)res[i]) % 10;
            res[i] = x + '0';
            t_max_msg_id_no += "9";
        }
        t_msg_id_no = res;
    } else {  // 否则对已有的消息 ID 进行递增处理,t_msg_id_no存在,且位置不全为9
        int i = t_msg_id_no.length() - 1;
        while (t_msg_id_no[i] == '9' && i >= 0) {
            i--;
        }
        if (i >= 0) {   // 说明存在不是 9 的位置,对该位置 +1, 后面进位变 0
            t_msg_id_no[i] += 1; 
            // 例如 23999 => 24000
            for (size_t j = i + 1; i < t_msg_id_no.length(); j++) {  // 后面的置为零
                t_msg_id_no[j] = '0';
            }
        }
    }
    return t_msg_id_no;
}
}  // namespace rocket
