#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "protocol.h"


/*
    本程序演示了共享内存+无名信号量实现进程间通信的安全写入。
    1. 创建共享内存对象
    2. 设置共享内存大小
    3. 内存映射
    4. 初始化无名信号量
    5. 循环写入数据，使用信号量保护临界区
*/
int main() {
    // 1. 创建共享内存对象
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open failed");
        exit(1);
    }

    // 2. 设置大小 (⚠️ 关键步骤)
    ftruncate(fd, sizeof(struct SharedPackage));

    // 3. 内存映射
    struct SharedPackage *ptr;
    ptr = (struct SharedPackage *)mmap(0, sizeof(struct SharedPackage),
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd, 0); //将共享内存映射到进程的虚拟地址空间，0表示让内核自动选择合适的地址
                                                        // PROT_READ | PROT_WRITE表示可读写
                                                        // MAP_SHARED表示多个进程可以共享这段内存
                                                        // fd是共享内存对象的文件描述符，最后一个0 offset表示从共享内存的起始位置开始映射
    if (ptr == MAP_FAILED) {    // 检查映射是否成功
        perror("mmap failed");
        exit(1);
    }

    sem_init(&ptr->mutex, 1, 1);

    printf("写端：初始化完成，开始发送数据...\n");

    int count = 0;
    while(1){
        sem_wait(&ptr->mutex); // P 加锁

        // 4. 写数据
        ptr->id = count;
        ptr->temperature = 20.0 + (rand() % 1000) / 50.0; // 随机温度
        snprintf(ptr->status, sizeof(ptr->status), "Status message %d", count);

        printf("写端：已写入数据 - ID: %d, Temp: %.2f, Status: %s\n",
               ptr->id, ptr->temperature, ptr->status);

        sem_post(&ptr->mutex); // v 解锁

        count++;
        sleep(1); // 每秒写一次
    }
    
// 清理（一般跑不到这里，除非按Ctrl+C处理后）
    sem_destroy(&ptr->mutex);
    munmap(ptr, sizeof(struct SharedPackage));
    close(fd);
    return 0;
}