#pragma once
#include <learnix/scheduler.h>

#define PIPE_BUF_SIZE 512

/* Kernel data structure of a pipe. Goes in struct file->ptr. */
struct pipe {
    #define PIPE_BUF_SIZE 512
    char buffer[PIPE_BUF_SIZE];
    struct wait_queue *rq;  // queue of blocked readers
    uint32_t nread;         // number of bytes read
    uint32_t nwrite;        // number of bytes written
    uint32_t nreaders;      // number of readers
    uint32_t nwriters;      // number of writers
};

/* Implementation of file_ops for a pipe. */
ssize_t pipe_read(struct file *f, void* buf, size_t count);
ssize_t pipe_write(struct file* f, void *buf, size_t count);
int pipe_close(struct file* f);
