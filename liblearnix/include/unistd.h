#pragma once
#include <sys/types.h>

ssize_t read(int fd, const void* buf, size_t len);
ssize_t write(int fd, const void* buf, size_t len);
int close(int fd);

pid_t getpid(void);
pid_t getppid(void);

int pipe(int pipefd[2]);

pid_t fork(void);
int execve(const char* pathname, const char** argv, const char **envp);
void exit(int status);
