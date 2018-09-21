#ifndef LOG_H
#define LOG_H

#ifndef MY_DEBUG       // µ÷ÊÔ¿ª¹Ø
#define MY_DEBUG
#endif

#ifdef MY_DEBUG
// Ìá¹©²Ù×÷½Ó¿Ú
#define Log(level) _global_logger_stream.setLoggerFunction(&Lib::_global_logger,&Lib::Logger::level).setLocation(__FILE__,__FUNCTION__,__LINE__)
#define Logf(level,format,args...) _global_logger.level(format, ##args)
#define TRACE trace
#define DEBUG debug
#define INFO info
#define WARN warn
#define ERROR error
#define CRITICAL critical

#else

#define Log(level)
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
#include <thread>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>

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
    std::shared_ptr<spdlog::logger> _getLogger(){
        time_t tnow = spdlog::log_clock::to_time_t(spdlog::log_clock::now());
        tm date = spdlog::details::os::localtime(tnow);

        std::ostringstream os;
        os << date.tm_year + 1900 << date.tm_mon + 1 << date.tm_mday << date.tm_hour << date.tm_min << date.tm_sec;
        std::string loggername = os.str();

        std::shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_mt(loggername);

        logger->set_pattern("[%l] [%Y-%m-%d %H:%M:%S] %v");
        logger->set_level(spdlog::level::trace);

        return logger;
    }

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
    }

    void setLoggerFunction(Logger *logger,loggerFuncType func){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_logger = logger;
        m_func = func;
    }

    void setLocation(const char* file,const char* function,int line){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_file = file;
        m_function = function;
        m_line = line;
    }

    void append(std::string&& str){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_buffer.append(std::move(str));
    }

    void start(){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_time != 0)
            return;

        _setCurrentTime();
        std::thread(thread_function,this).detach();
    }

    void flush(){
        std::ostringstream os;
        os << '[' << m_file << ':' << m_line << "] [" << m_function << "] ";
        std::string buffer = os.str();

        buffer.append(std::move(m_buffer));

        (m_logger->*(m_func))(buffer);

        m_buffer.clear();
        m_time = 0;
    }

    static void thread_function(LoggerTimer *timer){
        std::unique_lock<std::mutex> lock(timer->m_mutex);

        if(timer->m_time == 0){
            return;
        }

        while(1){
            if(timer->_getInterval() + timer->m_time < timer->_getCurrentTime()){
                timer->flush();
                return;
            }
            long interval = (timer->_getInterval() + timer->m_time - timer->_getCurrentTime());
            timer->m_cond_var.wait_for(lock, std::chrono::microseconds(interval));
            if(timer->_getCurrentTime() - timer->m_time >= timer->_getInterval()){
                if(timer->m_time != 0){
                    timer->flush();
                }
                return;
            }
        }
    }

    void stop(){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_time != 0){
            m_time = 0;
            m_cond_var.notify_one();
        }
    }

    void reset(){
        m_mutex.lock();
        if(m_time != 0){
            _setCurrentTime();
            m_mutex.unlock();
        }else{
            m_mutex.unlock();
            start();
        }
    }

private:
    long _getInterval(){ return 1000; } // 1000 us = 1 ms

    long _getCurrentTime(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return (tv.tv_sec % 10)*1000000 + tv.tv_usec;
    }

    void _setCurrentTime(){
        m_time = _getCurrentTime();
    }

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

    ostringstream os;
};

class LoggerStream
{
public:
    typedef void (Logger::*loggerFuncType)(const std::string&);

public:
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
};

struct _eol{
};

LoggerStream& operator<<(LoggerStream& os, const _eol &c){
    return os.flush();
}

template<typename OStream>
OStream &operator<<(OStream &os, const _eol &c){
}

// È«¾ÖÔ¤¶¨Òå±äÁ¿
_eol eol;
LoggerStream _global_logger_stream;
Logger _global_logger;

}

#endif // !MY_DEBUG

#endif // !LOG_H

