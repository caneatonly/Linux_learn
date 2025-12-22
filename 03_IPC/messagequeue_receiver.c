#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define Q_NAME "/my_priority_queue"
#define MAX_SIZE 1024

int main() {
    // 1. 打开队列
    mqd_t mq = mq_open(Q_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed (Run sender first?)");
        exit(1);
    }

    // 2. 获取属性 (为了知道 buffer 需要开多大)
    struct mq_attr attr;
    mq_getattr(mq, &attr);
    char *buf = malloc(attr.mq_msgsize); // 动态分配内存

    printf("接收端：开始接收...\n");

    while(1) {
        unsigned int priority;
        // 3. 接收消息 (会阻塞)
        ssize_t bytes = mq_receive(mq, buf, attr.mq_msgsize, &priority);
        
        if (bytes >= 0) {
            printf("收到: %s | 优先级: %u\n", buf, priority);
        } else {
            perror("recv error");
        }
    }
    
    // 清理
    free(buf);
    mq_close(mq);
    mq_unlink(Q_NAME);
    return 0;
}