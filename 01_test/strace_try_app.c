#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main() {
    printf("=== 程序开始 ===\n");
    
    // 试图打开一个不存在的文件
    int fd = open("/etc/not_exist_config.conf", O_RDONLY);
    
    if (fd < 0) {
        // 如果不用 perror，只打印 Failed，你就不知道为什么失败
        printf("打开配置文件失败！\n"); 
    } else {
        close(fd);
    }
    
    return 0;
}