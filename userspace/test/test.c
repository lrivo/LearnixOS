#include <stdio.h>
#define NAME_MAX_LEN 32

int main(void) {
    static char buffer[NAME_MAX_LEN + 1];

    printf("Name: ");
    ssize_t n = read(0, buffer, NAME_MAX_LEN);
    if (n == 1) {
        printf("Error occurred %d", -n);
        exit(1);
    }
    buffer[n] = '\0';

    printf("Read %ld bytes!\n", n);
    printf("Hello %s!\n", buffer);
    return 0;
}
