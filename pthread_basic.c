#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

// 与原例一致：根据你的板子实际路径调整
#define LED_BRIGHTNESS_PATH "/sys/class/leds/user-led/brightness"
#define LED_TRIGGER_PATH    "/sys/class/leds/user-led/trigger"

// 线程参数结构：用于把资源传给线程
typedef struct {
    int fd;               // brightness 的文件描述符
    useconds_t period_us; // 闪烁周期的一半（开/关各一次）
    int brightness_on;    // 开灯时写入值（通常 255 或 1）
    pthread_mutex_t *mtx; // 保护 period_us 的互斥锁
} blink_args_t;

static int set_trigger_none(void)
{
    int tfd = open(LED_TRIGGER_PATH, O_WRONLY);
    if (tfd >= 0) {
        ssize_t n = write(tfd, "none", 4);
        (void)n; // 简化：不强制检查 n
        close(tfd);
        return 0;
    }
    // 打不开可能是权限或路径问题，返回 errno 供上层参考
    return errno ? -errno : -1;
}

// 后台闪烁线程：循环写 brightness
static void* blink_thread(void* arg)
{
    blink_args_t* params = (blink_args_t*)arg;
    const int fd = params->fd;
    const int on = params->brightness_on;

    // 线程开始提示
    fprintf(stdout, "[blink] thread running (fd=%d)\n", fd);

    // 无限闪烁，直到进程结束（简单示例不加退出条件）
    while (1) {
        // 读取当前周期（受互斥锁保护）
        useconds_t half;
        if (params->mtx) {
            pthread_mutex_lock(params->mtx);
            half = params->period_us;
            pthread_mutex_unlock(params->mtx);
        } else {
            half = params->period_us;
        }
        // 开灯：确保从偏移 0 写入
        {
            char buf[16];
            int len = snprintf(buf, sizeof(buf), "%d", on); //将on转换成string并写入buf，返回len代表写入的字节数，
            // 使用 pwrite 固定偏移为 0，避免每次都 lseek
            (void)pwrite(fd, buf, (size_t)len, 0); //向fd对应的文件写入buf内容，长度为len，偏移量为0
        }
        usleep(half);

        // 关灯：写入 "0"
        (void)pwrite(fd, "0", 1, 0);
        usleep(half);
    }

    // 理论上不会到达
    return NULL;
}

int main(void)
{
    int fd = -1;

    // 1) 关闭 trigger，避免驱动自动控制与程序冲突
    int tr = set_trigger_none();
    if (tr == 0) {
        printf("Trigger disabled.\n");
    } else {
        // 容错：失败也继续，可能本来就是 none 或权限不够
        fprintf(stderr, "Warn: trigger not disabled (%d).\n", tr);
    }

    // 2) 打开 brightness 文件（读写）
    fd = open(LED_BRIGHTNESS_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open brightness file");
        printf("Did you verify the path? %s\n", LED_BRIGHTNESS_PATH);
        return EXIT_FAILURE;
    }

    printf("Starting multi-thread LED blink demo...\n");

    // 3) 创建后台线程：每秒打印/闪烁（500ms 开 + 500ms 关）
    blink_args_t args;
    pthread_mutex_t period_mtx = PTHREAD_MUTEX_INITIALIZER;
    args.fd = fd;
    args.period_us = 500 * 1000; // 500ms
    args.brightness_on = 255;    // 根据你的 max_brightness 调整（或用 1）
    args.mtx = &period_mtx;      // 用互斥锁保护周期变量

    pthread_t tid;
    int rc = pthread_create(&tid, NULL, blink_thread, &args);
    if (rc != 0) {
        fprintf(stderr, "pthread_create failed: %s\n", strerror(rc));
        close(fd);
        return EXIT_FAILURE;
    }

    // 4) 主线程：阻塞等待用户输入
    // 这模拟“右手画方”，而后台线程在“左手画圆”持续闪烁
    printf("Type milliseconds to set blink period (q to quit): \n");
    // 主线程读取用户输入，支持数值更新周期
    // 期望输入：如 200 表示 200ms（开 200ms + 关 200ms）
    char line[64];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        // 去掉尾部换行
        size_t L = strlen(line);
        if (L && line[L-1] == '\n') line[L-1] = '\0';
        if (line[0] == '\0') {
            continue;
        }
        if (line[0] == 'q' || line[0] == 'Q') {
            printf("Quit requested.\n");
            break;
        }
        char *endp = NULL;
        long ms = strtol(line, &endp, 10);
        if (endp == line || *endp != '\0' || ms <= 0 || ms > 10000) {
            printf("Invalid input. Enter positive milliseconds (1..10000) or q to quit.\n");
            continue;
        }
        useconds_t half_us = (useconds_t)ms * 1000; // 转微秒
        pthread_mutex_lock(&period_mtx);
        args.period_us = half_us;
        pthread_mutex_unlock(&period_mtx);
        printf("Blink period updated: %ld ms (on/off each).\n", ms);
        printf("Enter another value or q to quit: \n");
    }

    // 5) 资源回收（示例化简：不实现线程优雅退出）
    // 在严谨实现中，应该：
    // - 用原子变量/条件变量通知线程停止
    // - 线程退出前关灯
    // - pthread_join 等待线程结束

    // 尽管没有优雅退出，这里至少关闭 fd，避免泄漏
    close(fd);

    // 可选：取消并等待线程（实验性质，避免后台继续跑）
    // 注意：取消点行为取决于库，blink_thread 中 usleep 是取消点
    pthread_cancel(tid);
    pthread_join(tid, NULL);

    printf("Demo finished.\n");
    return EXIT_SUCCESS;
}
