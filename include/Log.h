#ifndef LOG_H
#define LOG_H

#ifndef MY_DEBUG       // 调试开关
#define MY_DEBUG
#endif

#ifdef MY_DEBUG
// 提供操作的宏接口
#define Log(level) _global_logger_stream.setLoggerFunction(&Lib::_global_logger,&Lib::Logger::level).setLocation(__FILE__,__FUNCTION__,__LINE__)
#define Logf(level,format,args...) _global_logger.level(format, ##args)
#define TRACE trace
#define DEBUG debug
#define INFO info
#define WARN warn
#define ERROR error
#define CRITICAL critical

#else

#define Log(level) _global_logger_stream
#define Logf(level,format,args...)
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
#include <sstream>
#include <condition_variable>
#include <mutex>
#endif //!MY_DUBEG

namespace Lib {
#ifdef MY_DEBUG
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
        m_logger->debug(msg);
    }

    template<typename T>
    void info(const T &msg){
        m_logger->info(msg);
    }

    template<typename T>
    void warn(const T &msg){
        m_logger->warn(msg);
    }

    template<typename T>
    void error(const T &msg){
        m_logger->error(msg);
    }

    template<typename T>
    void critical(const T &msg){
        m_logger->critical(msg);
    }

private:
    std::shared_ptr<spdlog::logger> _getLogger();

private:
    std::shared_ptr<spdlog::logger> m_logger;

};

class LoggerTimer
{
public:
    typedef void (Logger::*loggerFuncType)(const std::string&);

public:
    LoggerTimer():m_file(NULL),m_function(NULL),m_line(0),
        m_func(NULL),m_logger(NULL),
        m_time(0){
        }

    ~LoggerTimer(){
        stop();
        // Fix: 可能在线程退出之前就析构了，有访问野指针的风险
    }

    void setLoggerFunction(Logger *logger,loggerFuncType func);

    void setLocation(const char* file,const char* function,int line);

    void append(std::string&& str);

    void start();

    void flush();

    void stop();

    void reset();

private:
    long _getInterval(){ return 1000; } // 1000 us = 1 ms

    long _getCurrentTime();

    void _setCurrentTime(){ m_time = _getCurrentTime(); }

    static void thread_function(LoggerTimer *timer);

private:
    long m_time;

    std::condition_variable m_cond_var;

    std::mutex m_mutex;
    std::string m_buffer;

    const char* m_file;
    const char* m_function;
    int m_line;

    loggerFuncType m_func;
    Logger *m_logger;

    std::ostringstream os;
};

#endif // !MY_DEBUG

class LoggerStream
{
public:

#ifdef MY_DEBUG
    typedef void (Logger::*loggerFuncType)(const std::string&);

    LoggerStream& setLoggerFunction(Logger *logger,loggerFuncType func){
        m_timer.setLoggerFunction(logger,func);
        return *this;
    }

    LoggerStream& setLocation(const char* file,const char* function,int line){
        m_timer.setLocation(file,function,line);
        return *this;
    }

    LoggerStream& flush(){
        m_timer.flush();
        return *this;
    }

    template< typename T >
    LoggerStream& operator<<(const T& msg){
        fmt::memory_buffer w;
        fmt::format_to(w, SPDLOG_FILENAME_T("{}"), msg);
        m_timer.append(std::string(fmt::to_string(w)));
        m_timer.reset();
        return *this;
    }

private:
    LoggerTimer m_timer;
#else
    template< typename T >
    LoggerStream& operator<<(const T& msg){
        return *this;
    }

    LoggerStream& flush(){
        return *this;
    }
#endif  //!MY_DEBUG
};

struct _eol{
    friend LoggerStream& operator<<(LoggerStream& os, const _eol &c){
        return os.flush();
    }

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const _eol &c){}
};

// 全局预定义变量
extern _eol eol;
extern LoggerStream _global_logger_stream;

#ifdef MY_DEBUG
extern Logger _global_logger;
#endif

}

#endif // !LOG_H

