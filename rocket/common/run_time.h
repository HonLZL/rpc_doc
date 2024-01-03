#ifndef ROCKET_COMMON_RUN_TIME_H
#define ROCKET_COMMON_RUN_TIME_H
#include <string>

namespace rocket {
class RunTime {
    // rpc 运行时, 获得消息号,方法名,打印到日志

   public:
    static RunTime* GetRunTime();

   public:
    std::string m_msg_id;
    std::string m_method_name;
};
}  // namespace rocket

#endif
