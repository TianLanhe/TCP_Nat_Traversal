#ifndef LOG_H
#define LOG_H

#ifndef MY_DEBUG       // 调试开关
#define MY_DEBUG
#endif

#ifdef MY_DEBUG
// 提供操作接口
#define log(level) _global_logger_stream.setFunction(_global_logger::level)
#define TRACE trace
#define DEBUG debug
#define INFO info
#define WARN warn
#define ERROR error
#define CRITICAL critical

#else

#define log(level)
#define TRACE
#define DEBUG
#define INFO
#define WARN
#define ERROR
#define CRITICAL

#endif

#ifdef MY_DEBUG

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"

namespace Lib {

class Logger
{
public:

    Logger(){
        m_logger = _getLogger();
    }

    template<typename... Args>
    void trace(const char *fmt, const Args&... args){
        m_logger->trace(fmt,args...);
    }

    template<typename... Args>
    void debug(const char *fmt, const Args&... args){
        m_logger->debug(fmt,args...);
    }

    template<typename... Args>
    void info(const char *fmt, const Args&... args){
        m_logger->info(fmt,args...);
    }

    template<typename... Args>
    void warn(const char *fmt, const Args&... args){
        m_logger->warn(fmt,args...);
    }

    template<typename... Args>
    void error(const char *fmt, const Args&... args){
        m_logger->error(fmt,args...);
    }

    template<typename... Args>
    void critical(const char *fmt, const Args&... args){
        m_logger->critical(fmt,args...);
    }

    template<typename T>
    void trace(const T &msg){
        m_logger->trace(msg);
    }

    template<typename T>
    void debug(const T &msg){
        m_logger->trace(msg);
    }

    template<typename T>
    void info(const T &msg){
        m_logger->trace(msg);
    }

    template<typename T>
    void warn(const T &msg){
        m_logger->trace(msg);
    }

    template<typename T>
    void error(const T &msg){
        m_logger->trace(msg);
    }

    template<typename T>
    void critical(const T &msg){
        m_logger->trace(msg);
    }

private:
    std::shared_ptr<spdlog::logger> _getLogger(){
        std::conditional<std::is_same<spdlog::filename_t::value_type, char>::value, fmt::memory_buffer, fmt::wmemory_buffer>::type w;

        time_t tnow = spdlog::log_clock::to_time_t(spdlog::log_clock::now());
        tm date = spdlog::details::os::localtime(tnow);

        fmt::format_to(
            w, SPDLOG_FILENAME_T("{}{}{}{}{}{}"), date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour,date.tm_min,date.tm_sec);
        std::string loggername = fmt::to_string(w);

        std::shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_mt(loggername);

        logger->set_pattern("[%l] [%Y-%m-%d %H:%M:%S] %v");

        return logger;
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;

};

class LoggerStream
{
public:
    LoggerStream& setFunction()

private:
    void (CA::*PtrCaFuncTwo)(int ,int)

};

// 全局预定义变量
LoggerStream _global_logger_stream;
Logger _global_logger;

}

#endif // !MY_DEBUG

#endif // !LOG_H

