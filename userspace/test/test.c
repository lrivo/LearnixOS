#include <liblearnix.h>

int main() {
  int pipefd[2];
  char buffer[128];

  // create a pipe
  if (pipe(pipefd) == 0) {
    write(1, "sys_pipe OK!\n", 13);

    // write something in the pipe
    write(pipefd[1], "Hello!", 6);

    // read from the pipe
    int n = read(pipefd[0], buffer, 10);
    // print to stdout
    write(1, buffer, n); 
  }

  return 0;
}
