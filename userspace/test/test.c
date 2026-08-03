#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#define LEN 10

int main(void) {
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
