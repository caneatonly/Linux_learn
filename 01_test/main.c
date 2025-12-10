#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("=== STM32MP157 Linux Test ===\n");
    printf("Hello from VS Code + WSL2!\n");

    // 验证系统位数
    printf("Pointer size: %d bytes (Expected: 4)\n", sizeof(void*));

    // 简单的文件操作测试
    FILE *fp = fopen("/tmp/test_file.txt", "w");
    if (fp) {
        fprintf(fp, "Write from app at runtime!\n");
        fclose(fp);
        printf("File write test: SUCCESS (/tmp/test_file.txt)\n");
    } else {
        perror("File write failed");
    }

    return 0;
}