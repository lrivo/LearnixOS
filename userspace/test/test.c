#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#define LEN 10

int test_file_struct_sharing() {
    char buf[LEN + 1];
    char buf1[LEN + 1];

    int fd = open("/boot/ciao.txt", 0, 0);
    if (fd < 0) {
        printf("error: open %d\n", fd);
        exit(1);
    }
    
    printf("--- FD1 --- \n");
    int fd1 = open("/boot/ciao.txt", 0, 0);
    if (fd1 < 0) {
        printf("error: open fd1\n");
        exit(1);
    }
    
    // read 10 bytes on fd
    if (read(fd, buf, LEN) > 0)
        printf("buf = %s\n", buf);
    
    // read 10 bytes on fd1
    lseek(fd1, 6, SEEK_CURR);
    if (read(fd1, buf1, LEN) > 0)
        printf("buf1 = %s\n", buf1);

    return 0;
}

int test_vma_mmap() {
    char buf[10];

    void *addr = mmap((void*)0x500000, 4096, PROT_READ | PROT_WRITE, 0);    
    if (addr != 0x500000) {
        printf("error %d\n", (int)addr);
        return -1;
    }
    
    // try to read
    printf("read: \n");
    uint64_t *p = (uint64_t*)addr;
    printf("%d\n", p[22]);
    
    printf("write: \n");
    p[22] = 10;
    printf("%d\n", p[22]);
}

int main(void) {
    return test_vma_mmap();
}
