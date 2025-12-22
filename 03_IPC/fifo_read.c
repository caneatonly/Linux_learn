#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    char buf[128];

    // 1. 打开 FIFO
    int fd = open("/tmp/my_fifo", O_RDONLY);

    // 2. 循环读取
    while(1) {
        int len = read(fd, buf, sizeof(buf));
        if (len == 0) {
            printf("写端已断开，退出。\n");
            break;
        }
        buf[len] = '\0';
        printf("收到: %s\n", buf);
    }

    close(fd);
    return 0;
}