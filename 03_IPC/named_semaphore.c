#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    const char *sem_name = "/my_test_semaphore";
    
    // 创建有名信号量
    sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1); 
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }
    
    printf("✅ 有名信号量创建成功: %s\n", sem_name);
    printf("📍 挂载位置: /dev/shm%s\n", sem_name);
    printf("\n请在另一个终端查看:\n");
    printf("  ls -l /dev/shm/\n");
    printf("  ls -l /dev/shm%s\n", sem_name);
    printf("\n按 Enter 键删除信号量并退出...");
    
    getchar();  // 等待用户查看
    
    // 清理
    sem_close(sem);
    sem_unlink(sem_name);
    printf("\n信号量已删除\n");
    
    return 0;
}