#ifndef ROCKET_COMMON_MSG_UTIL_H
#define ROCKET_COMMON_MSG_UTIL_H
#include <string>

namespace rocket {
class MsgIdUtil {
   public:
    static std::string GenMsgId();
};

}  // namespace rocket

#endif