#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#define Q_NAME "/my_priority_queue"
#define MAX_SIZE 1024

int main() {
    // 1. 设置队列属性 (可选，但推荐设置)
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;      // 队列最多存10条
    attr.mq_msgsize = MAX_SIZE; // 每条最大1KB
    attr.mq_curmsgs = 0;

    // 2. 创建队列
    mqd_t mq = mq_open(Q_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        exit(1);
    }

    // 3. 发送普通消息 (优先级 1)
    for (int i = 1; i <= 3; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Normal Task %d", i);
        // 参数4: 优先级 = 1
        mq_send(mq, buf, strlen(buf) + 1, 1); 
        printf("发送: %s (Priority 1)\n", buf);
    }

    // 4. 发送紧急消息 (优先级 10) - 注意：这是最后发送的！
    char *urgent = "URGENT COMMAND: STOP!";
    mq_send(mq, urgent, strlen(urgent) + 1, 10);
    printf("发送: %s (Priority 10)\n", urgent);

    mq_close(mq);
    return 0;
}