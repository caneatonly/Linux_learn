#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef struct {
    uint32_t magic;      // 用于校验文件内容是否符合预期
    int32_t  id;
    double   value;
    char     name[16];   // 固定长度，便于二进制读写
} Record;

static void die_perror(const char* msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

static void print_record(const char *tag, const Record *r) {
    printf("%s\n", tag);
    printf("  magic = 0x%08X\n", r->magic);
    printf("  id    = %d\n", r->id);
    printf("  value = %.6f\n", r->value);
    printf("  name  = '%s'\n", r->name);
}

int main (void)
{
    const char *path = "record.bin";
    Record out={
        .magic = 0xDEADBEEF,
        .id    = 42,
        .value = 3.141592,
    };
    snprintf(out.name, sizeof(out.name), "%s", "hello-stdio");
        // 2) 写文件：wb（二进制写）
    FILE *fp = fopen(path, "wb");
    if (!fp) { die_perror("fopen(wb) failed"); return 1; }

    // fwrite 的语义：写 nmemb 个“元素”，每个元素 size 字节
    // 返回值：成功写入的“元素个数”（不是字节数）
    size_t written = fwrite(&out, sizeof(Record), 1, fp);
    if (written != 1) {
        if (ferror(fp)) die_perror("fwrite failed");
        else fprintf(stderr, "fwrite short write\n");
        fclose(fp);
        return 1;
    }

    if (fclose(fp) != 0) { die_perror("fclose(wb) failed"); return 1; }

    // 3) 读文件：rb（二进制读）
    fp = fopen(path, "rb");
    if (!fp) { die_perror("fopen(rb) failed"); return 1; }

    Record in;
    memset(&in, 0, sizeof(in));

    size_t readn = fread(&in, sizeof(Record), 1, fp);
    if (readn != 1) {
        if (feof(fp)) fprintf(stderr, "fread hit EOF early\n");
        if (ferror(fp)) die_perror("fread failed");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    // 4) 校验：magic + 内容对比
    if (in.magic != out.magic) {
        fprintf(stderr, "magic mismatch: got 0x%08X expected 0x%08X\n",
                in.magic, out.magic);
        return 1;
    }

    print_record("Record written:", &out);
    print_record("Record read back:", &in);

    // 结构体二进制写入，内存布局一致时可以做字节级比较
    if (memcmp(&out, &in, sizeof(Record)) == 0) {
        printf("OK: binary round-trip matches exactly.\n");
    } else {
        printf("WARN: round-trip differs at byte level (padding/FP/ABI?).\n");
    }

    return 0;
}