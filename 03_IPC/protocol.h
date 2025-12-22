#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <semaphore.h>

#define SHM_NAME "/my_safe_shm"

// 我们的数据包
struct SharedPackage {
    sem_t mutex;  // <--- 放在里面的信号量！(互斥锁)
    int id;
    float temperature;
    char status[64];
};

#endif