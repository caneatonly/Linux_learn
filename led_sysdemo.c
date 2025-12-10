#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// 宏定义路径：请确认你的板子路径是否一致
// 也可以使用 /sys/class/leds/user-led/brightness (这是软链接，指向同一个地方)
#define LED_BRIGHTNESS_PATH "/sys/class/leds/user-led/brightness"
#define LED_TRIGGER_PATH    "/sys/class/leds/user-led/trigger"

int main(void)
{
    int fd;
    
    // ============================================
    // 步骤 0: 这是一个好习惯 —— 先把 trigger 关掉
    // 否则驱动可能会和你的程序“打架”
    // ============================================
    fd = open(LED_TRIGGER_PATH, O_WRONLY);
    if (fd > 0) {
        write(fd, "none", 4);
        close(fd);
        printf("Trigger disabled.\n");
    }
    // 如果打开失败也没关系，可能已经是 none 或者权限问题，继续往下走

    // ============================================
    // 步骤 1: 打开 brightness 文件
    // ============================================
    fd = open(LED_BRIGHTNESS_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open brightness file");
        printf("Did you verify the path? %s\n", LED_BRIGHTNESS_PATH);
        return -1;
    }

    printf("Starting LED blink using Kernel Subsystem...\n");

    while (1) {
        // ============================================
        // 步骤 2: 开灯 (写入 255 或 1)
        // ============================================
        // 注意：write 接受的是字符串，不是整数
        write(fd, "255", 3); 
        
        // 既然是 Linux 应用，sleep 是让出 CPU 给其他进程，
        // 而不是像 MCU 那样死循环空转
        usleep(500 * 1000); // 延时 500ms

        // 重要！回绕文件指针
        lseek(fd, 0, SEEK_SET);

        // ============================================
        // 步骤 3: 关灯
        // ============================================
        write(fd, "0", 1);
        usleep(500 * 1000);

        lseek(fd, 0, SEEK_SET);
    }

    close(fd);
    return 0;
}