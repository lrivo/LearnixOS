#include <learnix/cpu.h>
#include <learnix/lib/string.h>
#include <learnix/lib/kprintf.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/syscall.h>
#include <learnix/fs.h>

/* Kernel data structure of a pipe. Goes in struct file->ptr. */
struct pipe {
    #define PIPE_BUF_SIZE 512
    char buffer[PIPE_BUF_SIZE];
    uint32_t nread;     // number of bytes read
    uint32_t nwrite;    // number of bytes written
};

/* File ops implementation for a pipe. */
ssize_t pipe_read
(struct file *f, void* buf, size_t count) {
    struct pipe *pipe;
    size_t n;
    
    kprintf("pipe_read\n");
    pipe = (struct pipe*)f->ptr;

    // pipe is empty, for now non-blocking behaviour
    if (pipe->nwrite == 0)
        return -1;

    // how much we can safely read from the pipe?
    n = pipe->nwrite - pipe->nread < count 
        ? pipe->nwrite - pipe->nread 
        : count;

    // read n bytes from the pipe into userspace
    // and advance the pipe's read counter
    memcpy(buf, &pipe->buffer[pipe->nread += n], n);
    return n;
}

ssize_t pipe_write
(struct file *f, void *buf, size_t count) {
    struct pipe *pipe;
    size_t n;
    
    kprintf("pipe_write\n");
    pipe = (struct pipe*)f->ptr;
    
    // pipe is full
    if (pipe->nwrite >= PIPE_BUF_SIZE)
        return -1;

    // how much we can safely write in the pipe?
    n = pipe->nwrite + count > PIPE_BUF_SIZE
        ? PIPE_BUF_SIZE - pipe->nwrite
        : count;

    // copy n bytes from userspace into the pipe
    // and advance the pipe's write counter
    memcpy(&pipe->buffer[pipe->nwrite += n], buf, n);
    return n;
}

static struct file_ops pipe_ops = { pipe_read, pipe_write, NULL };

/*
 *  Signature -> int pipe(int pipefd[2])
 *  
 *  Returns:
 *  pipefd[0] -> read-end of the pipe
 *  pipefd[1] -> write-end of the pipe
 *
 *  BUG: if pipefd[1] creation fails pipefd[0] is not cleaned
 *       this is a problem because struct process should use a
 *       struct file *fds[NFDS] instead of a struct file fds[NFDS]
 */
void sys_pipe
(struct intr_trap_frame *tf) {
    struct file *fd;
    struct process *p;
    struct pipe *pipe;
    int *pipefd, i;

    p = arch_cpu_get()->proc;
    pipefd = (int*)ARG0(tf);

    // 1. kmalloc a struct pipe
    pipe = kmalloc(sizeof(struct pipe));
    if (!pipe)
        goto bad;

    pipe->nread = 0;
    pipe->nwrite = 0;
    
    // 2. create the read file descriptor
    // 2.1 find the lowest usable file descriptor
    for (i = 0, fd = NULL; i < NFDS; i++)
    {
        if (p->fds[i].ptr == NULL)
        {
            fd = &p->fds[i];
            break;
        }
    }
    if (!fd)
        goto bad;
    
    // 2.2 initialize it
    fd->offset = 0;
    fd->refcount = 0;
    fd->ptr = (void*)pipe;
    fd->ops = &pipe_ops;
    pipefd[0] = i;
    kprintf("pipefd[0] = %d\n", i);

    // 3. create the write file descriptor
    // 3.1 find the lowest usable file descriptor
    for (i++, fd = NULL;i < NFDS; i++)
    {
        if (p->fds[i].ptr == NULL)
        {
            fd = &p->fds[i];
            break;
        }
    }
    if (!fd)
        goto bad;
    
    // 3.2 initialize it
    fd->offset = 0;
    fd->refcount = 0;
    fd->ptr = (void*)pipe;
    fd->ops = &pipe_ops;
    pipefd[1] = i;
    kprintf("pipefd[1] = %d\n", i);
    
    // 4. success
    RET(tf) = 0;
    return;

bad:
    if (pipe)
        kfree(pipe);
    RET(tf) = -1;
}
