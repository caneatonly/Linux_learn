#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


// 本程序用于演示死锁情况：两个线程互相等待对方持有的锁，导致程序卡死

pthread_mutex_t mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_B = PTHREAD_MUTEX_INITIALIZER;

void* thread_func_1(void* arg) {
    pthread_mutex_lock(&mutex_A);
    printf("[Thread 1] Got A, waiting for B...\n");
    sleep(1); // 确保线程2能运行并拿到B，诱发死锁
    
    pthread_mutex_lock(&mutex_B); // 这里会卡死
    printf("[Thread 1] Got A and B!\n");
    
    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);
    return NULL;
}

void* thread_func_2(void* arg) {
    pthread_mutex_lock(&mutex_B);
    printf("[Thread 2] Got B, waiting for A...\n");
    sleep(1); 
    
    pthread_mutex_lock(&mutex_A); // 这里会卡死
    printf("[Thread 2] Got B and A!\n");
    
    pthread_mutex_unlock(&mutex_A);
    pthread_mutex_unlock(&mutex_B);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func_1, NULL);
    pthread_create(&t2, NULL, thread_func_2, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}