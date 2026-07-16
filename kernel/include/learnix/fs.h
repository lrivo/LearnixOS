#pragma once
#include <learnix/types.h>

/* A file descriptor */
struct file
{
    // internal offset, not used by TTY
    uint32_t offset;
    uint32_t refcount;
    // underlying object (eg: tty, inode, pipe, ...)
    void *ptr;
    // vtable that implements read()/write()/... for each file type
    struct file_ops *ops;
};

/* A vtable of file operation function pointers */
struct file_ops
{
    ssize_t (*read)(struct file *f, void *buf, size_t count);
    ssize_t (*write)(struct file *f, void *buf, size_t count);
    int     (*close)(struct file *f);
};


/* A disk inode. Could be cached in memory. */
struct inode
{
    #define INODE_CACHED 1
    uint32_t flags;
    uint32_t inum;
};

// ==== FILE ==== //
/* Initializes a new file descriptor's kernel data structure. */
struct file *file_alloc(void *ptr, struct file_ops *ops);

/* Decrements the refcount and kfree(f) if it reaches zero. */
int file_close(struct file* f);
