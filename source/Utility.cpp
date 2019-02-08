#include "../include/Utility.h"
#include "../include/Exception.h"

#include <cstdlib>

using namespace std;
using namespace Lib;
using namespace Lib::Util;

/*#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>*/
#include <stdio.h>


#if defined(_WIN32) || defined(_WIN64)
#include <ctime>
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>
#endif

vector<string> Util::getLocalIps(){
    vector<string> ret;

	/*int s = socket(PF_INET, SOCK_DGRAM, 0);

    struct ifconf conf;
    char buff[BUFSIZ];
    conf.ifc_len = BUFSIZ;
    conf.ifc_buf = buff;

    if(ioctl(s, SIOCGIFCONF, &conf) == -1)
        return ret;

    int num = conf.ifc_len / sizeof(struct ifreq);
    struct ifreq *ifr = conf.ifc_req;

    for(int i=0;i < num;++i,++ifr){
        struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

        if(ioctl(s, SIOCGIFFLAGS, ifr) == -1)
            return ret;

        if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
            ret.push_back(string(inet_ntoa(sin->sin_addr)));
    }

    ::close(s);*/
    return ret;
}

int initRandSeed(){
#if defined(_WIN32) || defined(_WIN64)
	srand(time(NULL));
#elif defined(__linux__) || defined(__APPLE__)
    struct timeval tv;
    gettimeofday(&tv,NULL);
    srand(tv.tv_usec);
#endif
    return 0;
}

int nrand(int n){
    if(n <= 0){
        THROW_EXCEPTION(InvalidParameterException,"can not generate random num with the length equal to or smaller than 0");
    }

    static int r = initRandSeed();

    if(n > RAND_MAX)
        n = RAND_MAX;

    int bucket_size = RAND_MAX / n;
    do{
        r = rand() / bucket_size;
    }while( r >= n );

    return r;
}

int Util::getRandomNumByLength(int start,int length){
    if(length <= 0 || start >= RAND_MAX || start < 0){
        THROW_EXCEPTION(InvalidParameterException,"parameter error");
    }

    if(RAND_MAX - start < length)
        length = RAND_MAX - start;

    return nrand(length) + start;
}

int Util::getRandomNumByRange(int start, int end){
    if(start >= end || start >= RAND_MAX || start < 0){
        THROW_EXCEPTION(InvalidParameterException,"parameter error");
    }

    if(end > RAND_MAX){
        end = RAND_MAX;
    }

    return nrand(end-start) + start;
}

