#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <memory>
#include <queue>
#include <string>

#include "config.h"
#include "mutex.h"


// 命名空间（namespace）是用来组织代码并避免命名冲突的一种机制。
// 它允许你将一系列的标识符（变量名、函数名、类名等）封装在一个命名空间中，
// 从而避免与其他部分的代码发生命名冲突
// 使用时, rocket::Debug

namespace rocket {

// 模板实现
// 接受 str 和一个可变参数,这里的可变参数是 若干个格式化符号,%s %d 等
template <typename... Args>
std::string formatString(const char* str, Args&&... args) {
    int size = snprintf(nullptr, 0, str, args...);
    std::string result;
    if (size > 0) {
        result.resize(size);
        snprintf(&result[0], size + 1, str, args...);
    }
    return result;
}

// 反斜杠 \ 在这里表示宏定义在下一行继续，而不是在当前行结束。
// __VA_ARGS__宏用来接受不定数量的参数, 当__VA_ARGS__宏前面##时，可以省略参数输入
#define DEBUGLOG(str, ...)                                                                                   \
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString() + \
    "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" +                                  \
    rocket::formatString(str, ##__VA_ARGS__) + '\n');                                                        \
    rocket::Logger::GetGlobalLogger()->log();                                                                


#define INFOLOG(str, ...)                                                                                   \
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString() + \
    "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" +                                 \
    rocket::formatString(str, ##__VA_ARGS__) + '\n');                                                        \
    rocket::Logger::GetGlobalLogger()->log();         
    
#define ERRORLOG(str, ...)                                                                                   \
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString() + \
    "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" +                                  \
    rocket::formatString(str, ##__VA_ARGS__) + '\n');                                                        \
    rocket::Logger::GetGlobalLogger()->log();                                                                


enum LogLevel {
    UnKonwn = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};

class Logger {
   public:

    Logger(LogLevel level) : m_set_level(level) {}
    typedef std::shared_ptr<Logger> s_ptr;

    void pushLog(const std::string& msg);

    void log();

    // void log(LogEvent event);
    static Logger* GetGlobalLogger();
    static void InitGlobalLogger();

   private:
    LogLevel m_set_level;
    std::queue<std::string> m_buffer;

    Mutex m_mutex;
};

std::string LogLevelToString(LogLevel level);

LogLevel StringToLogLevel(const std::string& Log_level);

class LogEvent {
   public:
    LogEvent(LogLevel level)
        : m_level(level) {}

    std::string getFileName() const {
        return m_file_name;
    }
    LogLevel getLogLevel() const {
        return m_level;
    }

    std::string toString();

   private:
    std::string m_file_name;  // 文件名
    int32_t m_file_line;      // 行号
    int32_t m_pid;            // 进程号
    int32_t m_thread_id;      // 线程号

    LogLevel m_level;  // 日志级别
};

}  // namespace rocket


#endif