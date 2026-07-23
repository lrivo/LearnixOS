#include <liblearnix.h>

int main() {
  int pipefd[2];
  char buffer[128];

  // create a pipe
  if (pipe(pipefd) == -1) {
    write(1, "pipe failed\n", 12);
    exit(-1);
  }
  close(pipefd[0]); // close read end

  // fork
  int pid = fork();
  if (pid == 0) {
    // child
    ssize_t n = write(pipefd[1], "Hello from child!\n", 18);
    if (n == 18)
        write(1, "BAD\n", 4);
    if (n == -9)    // TODO: haven't implemented ERRNO yet, liblearnix propagates the syscall actual return values
        write(1, "ch EBADF\n", 10);
    if (n == -32)
        write(1, "ch EPIPE\n", 10);
  } else {
    // parent
    ssize_t n = read(pipefd[0], buffer, 20);
    if (n == -9)
        write(1, "pa EBADF\n", 10);
    if (n > 0) {
     write(1, "child says: ", 12);
     write(1, buffer, n);
    }
  }

  return 0;
}
