#include "../include/Semaphore.h"

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>

using namespace Lib;

Semaphore::Semaphore(int val):m_semid(-1){
    m_semid = semget(IPC_PRIVATE,1,IPC_CREAT|0660);

    if(m_semid != -1){
        semun arg;
        arg.val = val;
        semctl(m_semid,0,SETVAL,arg);
    }
}

Semaphore::~Semaphore(){
    if(m_semid != -1){
        semctl(m_semid,0,IPC_RMID);
    }
}

void Semaphore::request(){
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;  // P()
    sem_b.sem_flg = SEM_UNDO;

    semop(m_semid, &sem_b, 1);
}

void Semaphore::release(){
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;  // V()
    sem_b.sem_flg = SEM_UNDO;

    semop(m_semid, &sem_b, 1);
}
