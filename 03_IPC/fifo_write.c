#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

int main() {
    // 1. 创建 FIFO (如果已存在则忽略错误)
    if (mkfifo("/tmp/my_fifo", 0666) < 0) {
        // EEXIST 表示文件已存在，不是致命错误
    }

    // 2. 打开 FIFO (就像打开文件一样)
    // 注意：open 会阻塞，直到另一端也打开 FIFO 准备读
    printf("等待读端上线...\n");
    int fd = open("/tmp/my_fifo", O_WRONLY); 
    printf("读端已上线，开始发送...\n");

    char *msg = "This is data from Writer Process";
    while(1) {
        write(fd, msg, strlen(msg));
        sleep(1); // 每秒发一次
    }

    close(fd);
    return 0;
}