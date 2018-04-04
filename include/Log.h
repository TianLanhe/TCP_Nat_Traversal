#ifndef LOG_H
#define LOG_H

#ifndef MY_DEBUG
#define MY_DEBUG
#endif

#ifdef MY_DEBUG
#include <iostream>
#endif

namespace Lib {

template < typename T >
void log(const T& t){
#ifdef MY_DEBUG
    std::cout << t;
#endif
}

template < typename T, typename... Args >
void log(const T& t, Args... arg){
#ifdef MY_DEBUG
    log(t);
    log(arg...);
    if(sizeof...(arg) == 1)
        std::cout << std::endl;
#endif
}

}

#endif

