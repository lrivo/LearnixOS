#include <liblearnix.h>

int main() {
  int pipefd[2];
  char buffer[128];
  
  // create a pipe
  if (pipe(pipefd) == -1) {
    write(1, "pipe failed\n", 12);
    exit(-1);
  }
  
  // fork
  int pid = fork();
  if (pid == 0) {
    // child
    write(pipefd[1], "Hello from child!\n", 18);
  } else {
    // parent
    ssize_t n = read(pipefd[0], buffer, 20);
    if (n > 0) {
     write(1, "child says: ", 12);
     write(1, buffer, n);
    }
  }

  return 0;
}
