#include <stdio.h>

// 一个特意制造崩溃的函数
void create_crash() {
    int *ptr = NULL; // 定义一个空指针
    printf("1. 准备向空地址写入数据...\n");
    
    // 【崩溃点】试图往 0 地址写数据，这在 Linux 中是非法的
    *ptr = 100; 
    
    printf("2. 这行代码永远不会执行到。\n");
}

int main() {
    printf("=== 程序开始 ===\n");
    create_crash();
    printf("=== 程序结束 ===\n");
    return 0;
}