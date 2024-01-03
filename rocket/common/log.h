#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <pthread.h>
#include <semaphore.h>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "../net/timer.h"
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
#define DEBUGLOG(str, ...)                                                                                            \
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString() +          \
                                               "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + \
                                               rocket::formatString(str, ##__VA_ARGS__) + '\n');

#define INFOLOG(str, ...)                                                                                             \
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString() +           \
                                               "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + \
                                               rocket::formatString(str, ##__VA_ARGS__) + '\n');

#define ERRORLOG(str, ...)                                                                                            \
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString() +          \
                                               "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + \
                                               rocket::formatString(str, ##__VA_ARGS__) + '\n');

#define APPDEBUGLOG(str, ...)                                                                                            \
    rocket::Logger::GetGlobalLogger()->pushAppLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString() +          \
                                               "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + \
                                               rocket::formatString(str, ##__VA_ARGS__) + '\n');

#define APPINFOLOG(str, ...)                                                                                             \
    rocket::Logger::GetGlobalLogger()->pushAppLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString() +           \
                                               "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + \
                                               rocket::formatString(str, ##__VA_ARGS__) + '\n');

#define APPERRORLOG(str, ...)                                                                                            \
    rocket::Logger::GetGlobalLogger()->pushAppLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString() +          \
                                               "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + \
                                               rocket::formatString(str, ##__VA_ARGS__) + '\n');


enum LogLevel {
    UnKonwn = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};


// 异步日志, 生产者消费者模型,
class AsyncLogger {
   public:
    typedef std::shared_ptr<AsyncLogger> s_ptr;
    // 普通,不能有,临时对象是右值,普通引用不能传右值,所以改为 const
    // AsyncLogger(std::string& file_name, std::string file_path, int max_size);
    AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size);

    static void* Loop(void*);  // 需要将 void* 转换为实际类型再使用

    void stop();

    // 刷新到磁盘
    void flush();

    void pushLogBuffer(std::vector<std::string>& vec);

   private:
    //  m_file_path/m_file_name_yyyymmdd.0 , 从 0 开始,后面是 .1  .2  ...
    std::queue<std::vector<std::string>> m_buffer;

    std::string m_file_name;  // 日志输出文件名字
    std::string m_file_path;  // 日志输出路径
    int m_max_file_size{0};   // 单个日志文件的最大大小

    sem_t m_sempahore;
    pthread_t m_thread;

    pthread_cond_t m_condtion;  // 条件变量,要和互斥锁一起用
    Mutex m_mutex;              // 条件变量需要依赖锁来保护共享数据，同时也要在条件不满足时等待

    std::string m_date;             // 上次打印日志的文件日期
    FILE* m_file_handler{nullptr};  // 当前打开的日志文件句柄
    bool m_reopen_flag{false};      // 日否要重新打开该日志

    int m_no{0};  // 日志序号

    bool m_stop_flag{false};
};

class Logger {
   public:
    typedef std::shared_ptr<Logger> s_ptr;

    Logger(LogLevel level);

    void pushLog(const std::string& msg);
    void pushAppLog(const std::string& msg);

    void init();

    void log();

    // void log(LogEvent event);
    static Logger* GetGlobalLogger();
    static void InitGlobalLogger();

    void syncLoop();

   private:
    LogLevel m_set_level;
    std::vector<std::string> m_buffer;
    std::vector<std::string> m_app_buffer;

    Mutex m_mutex;

    // 可以用同一个锁,但是会引入多余的竞争
    Mutex m_app_mutex;

    std::string m_file_name;  // 日志输出文件名字
    std::string m_file_path;  // 日志输出路径
    int m_max_file_size{0};   // 单个日志文件的最大大小

    // rpc 调试 和 app 是两个线程打印
    AsyncLogger::s_ptr m_async_logger;

    AsyncLogger::s_ptr m_async_app_logger;

    TimerEvent::s_ptr m_timer_event;
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

    AsyncLogger::s_ptr m_async_logger;
};


}  // namespace rocket

#endif