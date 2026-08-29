#include <learnix/fs.h>
#include <learnix/cpu.h>
#include <learnix/syscall.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/lib/string.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/limine_module.h>

/* System-wide table of opened inodes, that can be reused by many file structs. */
static struct inode *itable[10] = { 0 };

/* File Operations table for the inode ramfs */
struct file_ops inode_ramfs_ops = {
    .read = inode_read, .write = NULL, .close = inode_close, .lseek = inode_lseek
};

/* File Operations implementation. */
ssize_t inode_read
(struct file *f, void *buf, size_t count) {
    struct inode *inode = (struct inode*)f->ptr;
    size_t n;
    
    // handle the End Of File case
    if (f->offset >= inode->size)
        return 0;
    
    // how many bytes we can safely read
    n = f->offset + count <= inode->size
        ? count
        : inode->size - f->offset;
    
    // to the copy and advance the pointer
    ssize_t r = arch_copy_to_user(buf, (void*)(inode->data + f->offset), n); 
    if (r < 0)
	return r;

    f->offset += n;
    return (ssize_t)n;
}

ssize_t  inode_write
(struct file *f, void *buf, size_t count) {
    struct inode *inode = (struct inode*)f->ptr;
    return 0;
}

/* Called by file_close(). Tries to kfree the underlying inode. */
int inode_close
(struct file *f) {
    struct inode *inode = (struct inode*)f->ptr;
    if (--inode->refcount == 0) {
        // remove from the itable
        for (int i = 0; i < 10; i++) {
            if (itable[i] == inode) itable[i] = NULL;
        } 
        // and then free the heap memory 
        kfree(inode);
        return 1;
    }
    return 0;
}

off_t inode_lseek
(struct file *f, off_t offset, int whence) {
    struct inode *inode = (struct inode*)f->ptr;

    switch (whence) {
        case SEEK_SET: {
            if (f->offset < 0)
                goto bad;
            f->offset = offset;
            break;
        }
        case SEEK_CURR: {
            f->offset += offset;
            break;
        }
        case SEEK_END: {
            f->offset = inode->size + offset;
            break;
        }
        default:
            goto bad;
    }
    return f->offset;

bad:
    return -1;
}

struct inode*
inode_create(const char *path) {
    struct inode* inode = NULL;

    struct limine_file* f = limine_module_get(path);
    kprintf("limine_module_get = %p\n", f);
    if (f) {
        // try searching the itable
        for (int i = 0; i < 10; i++) {
            if (itable[i] && itable[i]->data == (vaddr_t)f->address) {
                inode = itable[i];
                inode->inum = i;
                inode->refcount++;
                return inode;
            }
        }

        // otherwise, create it
        for (int i = 0; i < 10; i++) {
            // first unused entry
            if (!itable[i]) {
                inode = kmalloc(sizeof(struct inode));
                inode->data = (vaddr_t)f->address;
                inode->size = f->size;
                inode->inum = i;
                inode->refcount = 1;
                itable[i] = inode;
                break;
            }
        }
    }
    
    // this will be NULL when the first if or the two for fails
    // and !NULL when the second for succed
    return inode;
}
