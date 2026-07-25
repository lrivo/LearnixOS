#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NAME_MAX_LEN 32

int main(void) {
    static char buffer[NAME_MAX_LEN + 1];

    const char *mm = "Should never be printed!\n";
    if (write(stdin, mm, strlen(mm)) == -9) {
        printf("OK, stdin cannot be used to write\n");
    }

    printf("Name: ");
    ssize_t n = read(stdin, buffer, NAME_MAX_LEN);
    if (n == 1) {
        printf("Error occurred %d", -n);
        exit(1);
    }
    // discard the newline
    buffer[n - 1] = '\0';

    // duplicate stdout
    printf("== dup() test ==\n");
    int stdout1 = dup(1);
    if (stdout1 < 0) {
        printf("dup failed\n");
        exit(1);
    } else {
        printf("dup(1) = %d\n", stdout1);
    }

    printf("printf (fd 1) => %s\n", buffer);
    write(stdout1, buffer, n - 1);

    printf("\n== dup2() test ==\n");
    int stdout2 = dup2(stdout1, 5);
    if (stdout2 < 0) {
        printf("dup2 failed\n");
        exit(1);
    } else {
        printf("dup2() = %d\n", stdout2);
    }

    char *msg = "dup2 works!\n";
    write(stdout2, msg, strlen(msg));

    printf("== dup2 already opened ==\n");
    dup2(0, stdout2);
    write(stdout2, msg, strlen(msg));   // this should fail, as 0 is read-only

    return 0;
}
