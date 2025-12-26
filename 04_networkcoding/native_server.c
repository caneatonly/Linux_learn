#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERV_PORT 9527

int main(void) {
    int lfd, cfd; // lfd 监听socket ； cfd 服务socket
    struct sockaddr_in serv_addr, clit_addr;
    socklen_t clit_addr_len; /// 用于存放客户端地址信息
    char buf[BUFSIZ]; //BUFSIZE： 标准I/O缓冲区的默认大小
    int n, i;

    // 1. 创建 socket
    lfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET 表示使用 IPv4 协议;SOCK_STREAM 表示使用 TCP 协议;0 表示 使用默认协议 （字节流就用TCP，数据报就用UDP）
    if (lfd == -1) {
        perror("socket error");
        exit(1);
    }

    // 2. 初始化地址结构体
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_port = htons(SERV_PORT); // 端口转大端， htons: host to network short: 主机字节序转换为网络字节序（小端序转换为大端序）short 代表转换的是16位二进制数，端口是16位
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 接收任意IP htonl：host to network long : 主机字节序转换为网络字节序（小端序转换为大端序）long 代表转换的是32位二进制数 IP地址是32位

    // 3. 绑定
    // 加上这一行解决 Address already in use 问题
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // SOL_SOCKET：表示操作的是套接字层的选项，SO_REUSEADDR：允许重用本地地址
    //允许重用地址解决的问题： 当关闭服务器的时候，服务器端口会被内核锁定2min（TIME_WAIT状态），在开发阶段就必须等2min，允许地址复用的话就允许端口被重用

    if (bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind error");
        exit(1);
    }

    // 4. 监听
    // listen 将lfd标记为被动socket，用于接受传入连接请求
    if (listen(lfd, 128) == -1) { // 第二个参数是监听队列的最大长度，也就是最多允许128个客户端完成三次握手，等待服务器 accept
        perror("listen error");
        exit(1);
    }

    printf("等待客户端连接...\n");

    // 5. 接受连接
    clit_addr_len = sizeof(clit_addr);
    // 程序会阻塞在这里，直到有客户端连上来
    cfd = accept(lfd, (struct sockaddr *)&clit_addr, &clit_addr_len);
    if (cfd == -1) { // cfd = -1 表明接受连接失败
        perror("accept error");
        exit(1);
    }

    // 打印客户端信息
    char client_ip[64]; // 用于存放客户端IP地址
    printf("客户端连接成功: IP: %s, Port: %d\n",
           inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
           ntohs(clit_addr.sin_port));

    // 6. 通信循环
    while (1) {
        // 阻塞读取数据
        n = read(cfd, buf, sizeof(buf));// sizeof(buf) 表示缓冲区的大小

        if (n == -1) {
            perror("read error");
            break;
        } else if (n == 0) {
            // 重点！返回0代表对方关闭了连接
            printf("客户端已断开连接。\n");
            break;
        }

        printf("收到数据: %.*s\n", n, buf); // 打印收到的内容

        // 小写转大写业务逻辑
        for (i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }

        // 发回给客户端
        write(cfd, buf, n);
    }

    // 7. 关闭连接
    close(cfd); // 挂断当前电话
    close(lfd); // 关门歇业

    return 0;
}