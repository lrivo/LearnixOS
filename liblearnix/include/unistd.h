#pragma once
#include <sys/types.h>

ssize_t read(int fd, const void* buf, size_t len);
ssize_t write(int fd, const void* buf, size_t len);
int close(int fd);

#define SEEK_SET 0
#define SEEK_CURR 1
#define SEEK_END 2
off_t lseek(int fd, off_t offset, int whence);

pid_t getpid(void);
pid_t getppid(void);

int pipe(int pipefd[2]);
int yield(void);
int dup(int oldfd);
int dup2(int oldfd, int newfd);

pid_t fork(void);
int execve(const char* pathname, const char** argv, const char **envp);
void exit(int status);
