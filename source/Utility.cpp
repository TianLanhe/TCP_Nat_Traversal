#include "../include/Utility.h"
#include "../include/Exception.h"

#include <ctime>
#include <cstdlib>

using namespace Lib;
using namespace Lib::Util;

int initRandSeed(){
    srand(time(NULL));
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
    if(start <= end || start >= RAND_MAX || start < 0){
        THROW_EXCEPTION(InvalidParameterException,"parameter error");
    }

    if(end > RAND_MAX){
        end = RAND_MAX;
    }

    return nrand(end-start) + start;
}

