#ifndef SEMAPHORE_H
#define SEMAPHORE_H

namespace Lib{

class Semaphore
{

public:
    Semaphore(int val = 0);
    ~Semaphore();

    void request();

    void release();

private:
    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };

private:
    Semaphore& operator=(const Semaphore&);
    Semaphore(const Semaphore&);

private:
    int m_semid;
};

}

#endif // SEMAPHORE_H
