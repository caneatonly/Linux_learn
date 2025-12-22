#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "protocol.h"

int main() {
    // 1. 打开共享内存
    int fd = shm_open(SHM_NAME, O_RDWR, 0666); // 注意：读端也需要 O_RDWR，因为我们要修改信号量(减1)
    if (fd == -1) {
        perror("shm_open failed");
        exit(1);
    }
    
    struct SharedPackage *ptr;
    ptr = (struct SharedPackage *)mmap(0, sizeof(struct SharedPackage),
                                       PROT_READ | PROT_WRITE, // 需要写权限来操作信号量
                                       MAP_SHARED, fd, 0);

    printf("读端：已连接，准备读取...\n");

    while(1) {
        // === P操作 (加锁) ===
        // 如果写端正拿着锁，我会在这里死等
        sem_wait(&ptr->mutex);

        // 临界区：安全读取
        printf("--> 读到：ID=%d, Temp=%.2f, Status=%s\n", 
               ptr->id, ptr->temperature, ptr->status);
        
        // === V操作 (解锁) ===
        sem_post(&ptr->mutex);

        usleep(500 * 1000); // 0.5秒读一次，比写端快，验证会不会读到重复数据
    }

    munmap(ptr, sizeof(struct SharedPackage));
    close(fd);
    return 0;
}