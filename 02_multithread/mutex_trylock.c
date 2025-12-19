#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h> // 必须包含，用于使用 EBUSY


// 本程序用于演示 pthread_mutex_trylock 的用法： 尝试获取锁，失败则不阻塞，直接返回


pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER; // 静态初始化宏

// 霸道的计算线程：长时间持有锁
void* heavy_worker(void* arg) {
    while(1) {
        pthread_mutex_lock(&g_mutex);
        printf("[Worker] Doing heavy job (Lock held)...\n");
        sleep(3); // 模拟耗时操作，持有锁3秒
        pthread_mutex_unlock(&g_mutex);
        printf("[Worker] Released lock.\n");
        sleep(1); // 休息1秒
    }
    return NULL;
}

// 卑微的监控线程：尝试拿锁，拿不到就走
void* monitor_thread(void* arg) {
    while(1) {
        int ret = pthread_mutex_trylock(&g_mutex);
        if (ret == 0) {
            // 拿到锁了
            printf("[Monitor] Got lock! Updating status...\n");
            pthread_mutex_unlock(&g_mutex);
        } else if (ret == EBUSY) {
            // 锁被占用
            printf("[Monitor] Lock busy, skipping update.\n");
        }
        usleep(500000); // 每0.5秒尝试一次
    }
    return NULL;
}

// 先创建两个线程入口函数，在main中启动它们，与FreeRTOS类似
int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, heavy_worker, NULL);
    pthread_create(&t2, NULL, monitor_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}