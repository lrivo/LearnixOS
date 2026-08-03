#include <learnix/cpu.h>
#include <learnix/lib/string.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/scheduler.h>
#include <learnix/syscall.h>
#include <learnix/fs.h>
#include <learnix/pipe.h>

static struct file_ops pipe_read_fops = {
    .read=pipe_read, .write=NULL, .close=pipe_close
};

static struct file_ops pipe_write_fops = {
    .read=NULL, .write=pipe_write, .close=pipe_close
};

ssize_t pipe_read
(struct file *f, void* buf, size_t count) {
    struct pipe *pipe;
    size_t n;

    pipe = (struct pipe*)f->ptr;

    // pipe is empty with at least one writer
    if (pipe->nwrite == 0 && pipe->nwriters > 0) {
        sleep_on(&pipe->rq);
    }

    // how much we can safely read from the pipe?
    n = pipe->nwrite - pipe->nread < count
        ? pipe->nwrite - pipe->nread
        : count;

    // no writers left and nothing to read -> EOF
    if (pipe->nwriters == 0 && n == 0)
        return 0;

    // read n bytes from the pipe into userspace
    // and advance the pipe's read counter
    memcpy(buf, &pipe->buffer[pipe->nread += n], n);
    return n;
}

ssize_t pipe_write
(struct file *f, void *buf, size_t count) {
    struct pipe *pipe;
    size_t n;

    pipe = (struct pipe*)f->ptr;

    // pipe has no readers
    if (pipe->nreaders == 0)
        return -EPIPE;
    // pipe is full
    if (pipe->nwrite >= PIPE_BUF_SIZE)
        return -1;  // TODO: non Linux compliant

    // how much we can safely write in the pipe?
    n = pipe->nwrite + count > PIPE_BUF_SIZE
        ? PIPE_BUF_SIZE - pipe->nwrite
        : count;

    // copy n bytes from userspace into the pipe
    // and advance the pipe's write counter
    memcpy(&pipe->buffer[pipe->nwrite += n], buf, n);

    // wake up potential readers
    wake_up(&pipe->rq);
    return n;
}

int pipe_close
(struct file *f) {
    struct pipe* pipe = (struct pipe*)f->ptr;

    if (f->ops == &pipe_read_fops) {
        // decrement the readers counter
        pipe->nreaders--;
    } else {
        // decrement the writers counter
        pipe->nwriters--;
        // wake-up potential readers who are blocked
        if (pipe->nreaders > 0)
            wake_up(&pipe->rq);
    }

    if (pipe->nreaders == 0 && pipe->nwriters == 0) {
        kfree(pipe);
    }

    return 0;
}

/* Helpers */
// Returns the lowest usable file descriptor, -1 on failure.
static int fd_find(struct process *p) {
    for (int i = 0; i < NFDS; i++)
        if (!p->fds[i]) return i;
    return -1;
}

/*
 *  Signature -> int pipe(int pipefd[2])
 *
 *  Returns:
 *  pipefd[0] -> read-end of the pipe
 *  pipefd[1] -> write-end of the pipe
 *
 *  If it fails it must never modify the pipefd argument.
 */
void sys_pipe
(struct intr_trap_frame *tf) {
    struct file *fd1 = NULL, *fd2 = NULL;
    struct pipe *pipe = NULL;
    struct process *p;
    int *pipefd, i, j;

    p = arch_cpu_get()->proc;
    pipefd = (int*)ARG0(tf);

    // 1. allocate the pipe's kernel data structure, zero-ed out.
    pipe = kzalloc(sizeof(struct pipe));
    if (!pipe)
        goto bad;
    else
        pipe->nreaders = pipe->nwriters = 1;

    // 2. create the read file descriptor
    // 2.1 find the lowest usable file descriptor
    i = fd_find(p);
    if (i < 0)
        goto bad;

    // 2.2 initialize it as a read-only file descriptor
    fd1 = file_alloc(pipe, &pipe_read_fops);
    if (!fd1)
        goto bad;
    p->fds[i] = fd1;

    // 3. create the write file descriptor
    j = fd_find(p);
    if (j < 0)
        goto bad;

    fd2 = file_alloc(pipe, &pipe_write_fops);
    if (!fd2)
        goto bad;
    p->fds[j] = fd2;

    /* 4. now we can modify p->fds[]. We do it at the end because per POSIX
     * specs, if pipe() fails, it must not modify the pipefd argument. */
    pipefd[0] = i;
    pipefd[1] = j;
    RET(tf) = 0;
    return;

bad:
    if (pipe)
        kfree(pipe);
    if (fd1) {
        p->fds[i] = NULL;
        kfree(fd1);
    }
    if (fd2) {
        p->fds[j] = NULL;
        kfree(fd2);
    }
    RET(tf) = -1;
}
