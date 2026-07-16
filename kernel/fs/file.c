/*
 *  file.c contains various helper functions to manage the lifecycle
 *  of a file descriptor in the kernel, represented by struct file.
 */
#include <learnix/fs.h> 
#include <learnix/mm/kmalloc.h>

struct file*
file_alloc(void *ptr, struct file_ops *ops) {
    struct file *f = kmalloc(sizeof(struct file));
    if (f) {
        f->offset = 0;
        f->refcount = 1;
        f->ptr = ptr;
        f->ops = ops;
    }
    return f;
}

int
file_close(struct file *f) {
    // avoid a NULL pointer dereference later
    if (!f) 
        return -1;

    /* decrement refcount, and close the file if
       it reaches zero. */
    if (--f->refcount == 0) {
        // if possible, call this file_ops->close() function
        if (f->ops->close)
            f->ops->close(f);

        // and then free this file
        kfree(f);
        return 1;
    }

    // file existed but nothing closed
    return 0;
}
