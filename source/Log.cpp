#include "../include/Log.h"

#include <thread>
#include <sys/time.h>

using namespace std;
using namespace Lib;

_eol Lib::eol;
LoggerStream Lib::_global_logger_stream;
Logger Lib::_global_logger;

shared_ptr<spdlog::logger> Logger::_getLogger(){
    time_t tnow = spdlog::log_clock::to_time_t(spdlog::log_clock::now());
    tm date = spdlog::details::os::localtime(tnow);

    ostringstream os;
    os << date.tm_year + 1900 << date.tm_mon + 1 << date.tm_mday << date.tm_hour << date.tm_min << date.tm_sec;
    string loggername = os.str();

    shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_mt(loggername);

    logger->set_pattern("[%l] [%Y-%m-%d %H:%M:%S] %v");
    logger->set_level(spdlog::level::trace);

    return logger;
}

void LoggerTimer::setLoggerFunction(Logger *logger,loggerFuncType func){
    unique_lock<mutex> lock(m_mutex);
    m_logger = logger;
    m_func = func;
}

void LoggerTimer::setLocation(const char* file,const char* function,int line){
    unique_lock<mutex> lock(m_mutex);
    m_file = file;
    m_function = function;
    m_line = line;
}

void LoggerTimer::append(string&& str){
    unique_lock<mutex> lock(m_mutex);
    m_buffer.append(move(str));
}

void LoggerTimer::start(){
    unique_lock<mutex> lock(m_mutex);
    if(m_time != 0)
        return;

    _setCurrentTime();
    thread(thread_function,this).detach();
}

void LoggerTimer::flush(){
    ostringstream os;
    os << '[' << m_file << ':' << m_line << "] [" << m_function << "] ";
    string buffer = os.str();

    buffer.append(m_buffer);

    (m_logger->*(m_func))(buffer);

    m_buffer.clear();
    m_time = 0;
}

void LoggerTimer::thread_function(LoggerTimer *timer){
    unique_lock<mutex> lock(timer->m_mutex);

    if(timer->m_time == 0){
        return;
    }

    while(1){
        if(timer->_getInterval() + timer->m_time < timer->_getCurrentTime()){
            timer->flush();
            return;
        }
        long interval = (timer->_getInterval() + timer->m_time - timer->_getCurrentTime());
        timer->m_cond_var.wait_for(lock, chrono::microseconds(interval));
        if(timer->_getCurrentTime() - timer->m_time >= timer->_getInterval()){
            if(timer->m_time != 0){
                timer->flush();
            }
            return;
        }
    }
}

void LoggerTimer::stop(){
    unique_lock<mutex> lock(m_mutex);
    if(m_time != 0){
        m_time = 0;
        m_cond_var.notify_one();
    }
}

void LoggerTimer::reset(){
    m_mutex.lock();
    if(m_time != 0){
        _setCurrentTime();
        m_mutex.unlock();
    }else{
        m_mutex.unlock();
        start();
    }
}

long LoggerTimer::_getCurrentTime(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec % 10)*1000000 + tv.tv_usec;
}

/*LoggerStream& Lib::operator<<(LoggerStream& os, const _eol &c){
    return os.flush();
}

template<typename OStream>
OStream& Lib::operator<<(OStream &os, const _eol &c){}*/
