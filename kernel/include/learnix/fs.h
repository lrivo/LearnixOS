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

/* A ramfs file */
struct inode
{
    vaddr_t data;   // actual bytes of the file
    uint64_t size;  // siize of the file
    uint32_t inum;
    uint32_t refcount;
};

// ==== FILE ==== //
/* Initializes a new file descriptor's kernel data structure. */
struct file *file_alloc(void *ptr, struct file_ops *ops);

/* Decrements the refcount and kfree(f) if it reaches zero. */
int file_close(struct file* f);

// ==== INODE ==== //
struct inode* inode_create(const char* path);
ssize_t inode_read(struct file *f, void *buf, size_t count);
ssize_t inode_write(struct file *f, void *buf, size_t count);
int inode_close(struct file *f);
