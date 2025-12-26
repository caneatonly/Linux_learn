#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

// 端口定义
#define SERVER_PORT 8888

// 1. 定义协议头 (必须 1 字节对齐)
#pragma pack(1)
struct ProtocolHeader {
    uint32_t pkt_type;  // 0=文本, 1=图片
    uint32_t body_len;  // 数据体长度
};
#pragma pack()

// ==========================================
// 核心挑战 1: 实现 readn
// ==========================================
/**
 * 这是一个死磕到底的读取函数
 * @param fd: 套接字描述符
 * @param vptr: 接收缓冲区指针
 * @param n: 必须要读到的字节数
 * @return: 成功返回 n，对方关闭返回 0 (或实际读到的 < n)，出错返回 -1
 */
ssize_t readn(int fd, void *vptr, size_t n) {
    size_t  nleft;      // 还需要读多少
    ssize_t nread;      // 本次 recv 读到了多少
    char    *ptr;       // 当前读到缓冲区的哪个位置了

    ptr = (char *)vptr; // 转换指针类型方便计算
    nleft = n;          // 初始化还需要读 n 个

    while (nleft > 0) {
        // TODO: 请在这里填写代码
        // 1. 调用 recv，注意处理 EINTR 信号中断的情况
        // 2. 处理 recv 返回 0 (对方关闭连接) 的情况
        // 3. 更新 nleft 和 ptr 指针
    }
    return (n - nleft); // 返回实际读取的总字节数
}

int main(void) {
    // 省略 socket(), bind(), listen() 的样板代码...
    // 假设我们已经 accept 成功，得到了 connfd
    int listenfd, connfd;
    struct sockaddr_in servaddr;
 
    // --- 快速搭建服务器环境 (模拟) ---
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);
    
    // 为了方便测试，加个端口复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 5);
    
    printf("等待客户端连接中 (请启动测试客户端)...\n");
    connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
    printf("客户端已连接！准备接收数据...\n");

    // ==========================================
    // 核心挑战 2: 业务接收循环
    // ==========================================
    struct ProtocolHeader header;
    char *body = NULL;

    while (1) {
        // Step A: 读取头部
        // TODO: 调用 readn 读取 sizeof(header) 个字节
        // 如果返回值 == 0，打印 "Client closed" 并 break
        // 如果返回值 < sizeof(header)，打印 Error 并 break
        
        // Step B: 字节序转换 (网络序 -> 主机序)
        // TODO: 使用 ntohl 处理 header.pkt_type 和 header.body_len

        printf("DEBUG: 收到包头 [Type=%d, Len=%d]\n", header.pkt_type, header.body_len);

        // Step C: 读取数据体
        if (header.body_len > 0) {
            body = (char *)malloc(header.body_len + 1);
            if (body == NULL) break;

            // TODO: 调用 readn 读取 header.body_len 个字节到 body 中
            // 同样需要处理读取失败的情况

            body[header.body_len] = '\0'; // 补上结束符方便打印
            printf("RECV: %s\n\n", body);
            free(body);
        }
    }

    close(connfd);
    close(listenfd);
    return 0;
}