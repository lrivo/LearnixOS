#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#define LEN 5

int main(void) {
    char buf[LEN + 1];

    int fd = open("/boot/ciao.txt", 0, 0);
    if (fd < 0) {
        printf("error: open %d\n", fd);
        exit(1);
    }

    // parent reads 5 bytes
    if (read(fd, buf, LEN) == 5)
        printf("Parent has read 5 bytes!\n");

    // fork
    switch (fork()) {
        case -1: {
            printf("error: fork\n");
            exit(1);
        }
        case 0: {
            // child
            int n = read(fd, buf, LEN);
            printf("child has read %d bytes => %s\n", n, buf);
            break;
        }
        default: {
            wait(NULL);
        }
    }

    return 0;
}
