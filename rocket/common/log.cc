#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <sstream>

#include "../net/eventloop.h"
#include "config.h"
#include "log.h"
#include "mutex.h"
#include "run_time.h"
#include "util.h"

namespace rocket {

static Logger* g_logger = nullptr;
Logger* Logger::GetGlobalLogger() {
    return g_logger;
}

Logger::Logger(LogLevel level, int type /*=1*/)
    : m_set_level(level), m_type(type) {
    if (m_type == 0) {
        return;
    }
    m_async_logger = std::make_shared<AsyncLogger>(
        Config::GetGlobalConfig()->m_log_file_name + "_rpc",
        Config::GetGlobalConfig()->m_log_file_path,
        Config::GetGlobalConfig()->m_log_max_file_size);

    m_async_app_logger = std::make_shared<AsyncLogger>(
        Config::GetGlobalConfig()->m_log_file_name + "_app",
        Config::GetGlobalConfig()->m_log_file_path,
        Config::GetGlobalConfig()->m_log_max_file_size);
}

void Logger::init() {
    if (m_type == 0) {
        return;
    }

    m_timer_event = std::make_shared<TimerEvent>(Config::GetGlobalConfig()->m_log_sync_interval,
                                                 true, std::bind(&Logger::syncLoop, this));  // 成员函数传入需要有 this, 表示 this.syncLoop(), 如果是静态函数则不需要
    EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
}

void Logger::syncLoop() {
    // 同步 m_buffer 到 async_logger 的 buffer 队尾
    // printf("sync to async logger\n");
    // printf("m_buffer size = %ld,\t m_app_buffer size = %ld\n", m_buffer.size(), m_app_buffer.size());
    std::vector<std::string> tmp_vec;
    ScopeMutex<Mutex> lock(m_mutex);
    tmp_vec.swap(m_buffer);
    lock.unlock();

    // 添加到队尾
    if (!tmp_vec.empty()) {
        m_async_logger->pushLogBuffer(tmp_vec);
    }

    tmp_vec.clear();
    std::vector<std::string> tmp_vec2;
    ScopeMutex<Mutex> lock2(m_app_mutex);
    tmp_vec2.swap(m_app_buffer);
    lock2.unlock();

    // 添加到队尾
    if (!tmp_vec2.empty()) {
        m_async_app_logger->pushLogBuffer(tmp_vec2);
    }
    tmp_vec2.clear();
}

// 设置 日志级别, 并作为入参, 初始化 全局 g_logger,
void Logger::InitGlobalLogger(int type /*=1*/) {
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    printf("Init log level [%s] \n", LogLevelToString(global_log_level).c_str());

    g_logger = new Logger(global_log_level, type);
    g_logger->init();
}

// LogEvent(LogLevel level);

std::string LogLevelToString(LogLevel level) {
    switch (level) {
        case Debug:
            return "DEBUG";

        case Info:
            return "INFO";

        case Error:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}

LogLevel StringToLogLevel(const std::string& log_level) {
    if (log_level == "DEBUG")
        return Debug;
    else if (log_level == "INFO")
        return Info;
    else if (log_level == "ERROR")
        return Error;
    else
        return UnKonwn;
}

std::string LogEvent::toString() {
    struct timeval now_time;

    gettimeofday(&now_time, nullptr);

    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);

    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);

    std::string time_str(buf);

    int ms = now_time.tv_usec * 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    std::stringstream ss;

    ss << "[" << LogLevelToString(m_level) << "]\t"
       << "[" << time_str << "]\t"
       << "[" << m_pid << ":" << m_thread_id << "]\t";

    // 获取当前线程处理的请求的 msgid
    std::string msg_id = RunTime::GetRunTime()->m_msg_id;
    std::string method_name = RunTime::GetRunTime()->m_method_name;
    if (!msg_id.empty()) {
        ss << "[" << m_pid << ":" << m_thread_id << "]\t";
    }
    if (!method_name.empty()) {
        ss << "[" << method_name << "]\t";
    }

    return ss.str();
}

void Logger::pushLog(const std::string& msg) {
    if (m_type == 0) {
        printf((msg + "\n").c_str());
        return;
    }
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push_back(msg);
    lock.unlock();
}

void Logger::pushAppLog(const std::string& msg) {
    ScopeMutex<Mutex> lock(m_app_mutex);
    m_app_buffer.push_back(msg);
    lock.unlock();
}

void Logger::log() {
    // ScopeMutex<Mutex> lock(m_mutex);

    // std::vector<std::string> tmp;
    // m_buffer.swap(tmp);

    // lock.unlock();

    // while (!tmp.empty()) {
    //     std::string msg = tmp.front();
    //     tmp.pop();
    //     // cout << msg.c_str() << endl;
    //     printf("%s", msg.c_str());
    // }
}

AsyncLogger::AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size)
    : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size) {
    sem_init(&m_sempahore, 0, 0);
    assert(pthread_create(&m_thread, nullptr, &AsyncLogger::Loop, this) == 0);

    // assert(pthread_cond_init(&m_condtion, nullptr) == 0);

    sem_wait(&m_sempahore);
}

void* AsyncLogger::Loop(void* arg) {
    // 将 buffer 的全部数据全部打印到文件中,然后线程睡眠,直到有新的数据写入, 循环

    AsyncLogger* logger = reinterpret_cast<AsyncLogger*>(arg);

    assert(pthread_cond_init(&logger->m_condtion, nullptr) == 0);

    sem_post(&logger->m_sempahore);
    while (true) {
        ScopeMutex<Mutex> lock(logger->m_mutex);

        // 持续等待,直到有数据,被唤醒
        while (logger->m_buffer.empty()) {
            pthread_cond_wait(&(logger->m_condtion), logger->m_mutex.getMutex());
        }

        // 被唤醒后的操作
        std::vector<std::string> tmp;
        tmp.swap(logger->m_buffer.front());
        logger->m_buffer.pop();

        lock.unlock();

        timeval now;
        gettimeofday(&now, nullptr);

        struct tm now_time;
        localtime_r(&(now.tv_sec), &now_time);

        const char* format = "%Y%m%d";
        char date[32];
        strftime(date, sizeof(date), format, &now_time);

        // 日期改变了
        if (std::string(date) != logger->m_date) {
            logger->m_no = 0;
            logger->m_reopen_flag = true;
            logger->m_date = std::string(date);
        }

        if (logger->m_file_handler == nullptr) {
            logger->m_reopen_flag = true;
        }

        std::stringstream ss;
        ss << logger->m_file_path << logger->m_file_name << "_"
           << std::string(date) << "_log.";
        std::string log_file_name = ss.str() + std::to_string(logger->m_no);

        if (logger->m_reopen_flag) {
            if (logger->m_file_handler) {
                fclose(logger->m_file_handler);
            }
            logger->m_file_handler = fopen(log_file_name.c_str(), "a");
            logger->m_reopen_flag = false;
        }

        // 获取文件流(FILE*)的当前位置
        if (ftell(logger->m_file_handler) > logger->m_max_file_size) {
            fclose(logger->m_file_handler);

            log_file_name = ss.str() + std::to_string(logger->m_no++);
            logger->m_file_handler = fopen(log_file_name.c_str(), "a");
            logger->m_reopen_flag = false;
        }

        for (auto& i : tmp) {
            if (!i.empty()) {
                fwrite(i.c_str(), 1, i.length(), logger->m_file_handler);
            }
        }
        // 刷新
        fflush(logger->m_file_handler);

        if (logger->m_stop_flag) {
            return nullptr;
        }
    }

    return nullptr;
}

void AsyncLogger::stop() {
    m_stop_flag = true;
}

void AsyncLogger::flush() {
    fflush(m_file_handler);
}

void AsyncLogger::pushLogBuffer(std::vector<std::string>& vec) {
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(vec);
    pthread_cond_signal(&m_condtion);

    lock.unlock();

    // 这时候需要唤醒异步日志线程
    // printf("pthread_cond_signal\n");
}
}  // namespace rocket
