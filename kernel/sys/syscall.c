#include "learnix/arch/interrupts.h"
#include "learnix/fs.h"
#include "learnix/process.h"
#include <learnix/cpu.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>
#include <learnix/scheduler.h>
#include <learnix/syscall.h>
#include <learnix/tty.h>

extern struct tty_ctx tty0;

// syscall vtable
static void (*sys_table[128])(struct intr_trap_frame *tf) = {
  [SYS_READ] = sys_read,
  [SYS_WRITE] = sys_write,
  [SYS_OPEN] = sys_open,
  [SYS_CLOSE] = sys_close,
  [SYS_LSEEK] = sys_lseek,
  [SYS_MMAP] = sys_mmap,
  [SYS_MUNMAP] = sys_munmap,
  [SYS_PIPE] = sys_pipe,
  [SYS_YIELD] = sys_yield,
  [SYS_DUP] = sys_dup,
  [SYS_DUP2] = sys_dup2,
  [SYS_GETPID] = sys_getpid,
  [SYS_FORK] = sys_fork,
  [SYS_EXECVE] = sys_execve,
  [SYS_EXIT] = sys_exit,
  [SYS_WAIT] = sys_wait,
  /* --- non POSIX syscalls --- */
  [SYS_SET_FG_PROC] = sys_set_fg_proc,
  [SYS_SCHED_GET_STATS] = sys_sched_get_stats,
  [SYS_SCHED_SET_PRIO] = sys_sched_set_prio,
};

void
syscall_dispatcher (struct intr_trap_frame *tf)
{
  size_t num = NUM (tf);
  if (num < 128 && sys_table[num])
    sys_table[num](tf);
  else
    RET(tf) = -1;
}

void
sys_read (struct intr_trap_frame *tf)
{
  int fd = (int)ARG0(tf);
  void *buf = (void*)ARG1(tf);          // validated by arch_copy_to_user() in the file ops
  size_t count = (size_t)ARG2(tf);

  // make sure fd is a valid integer [0; NFDS-1]
  if (fd < 0 || fd >= NFDS) {
      RET(tf) = -EBADF;
  } else {
      struct file *f = arch_cpu_get()->proc->fds[fd];
      if (!f || !f->ops->read)
          RET(tf) = -EBADF; // f is either closed or not readable
      else
          RET(tf) = f->ops->read(f, buf, count);
  }
}

void
sys_write (struct intr_trap_frame *tf)
{
  int fd = (int)ARG0(tf);
  void *buf = (void*)ARG1(tf);          // validated by arch_copy_from_user() in the file ops
  size_t count = (size_t)ARG2 (tf);

  // make sure fd is a valid integer [0; NFDS-1]
  if (fd < 0 || fd >= NFDS) {
      RET(tf) = -EBADF;
  } else {
      struct file *f = arch_cpu_get()->proc->fds[fd];
      if (!f || !f->ops->write)
          RET(tf) = -EBADF; // f is either closed or not writable
      else
          RET(tf) = f->ops->write(f, buf, count);
  }
}

void
sys_open(struct intr_trap_frame *tf) {
    char path[PATH_MAX];
    struct process *proc;
    int fd;

    // pathname is a userspace pointer: copy it into kernel memory first
    if (arch_strncpy_from_user(path, (const char*)ARG0(tf), sizeof(path)) < 0) {
        RET(tf) = -EFAULT;
        return;
    }

    struct inode *inode = inode_create(path);
    kprintf("inode at %p with inum %u\n", inode, inode->inum);
    kprintf("file data at %p with size %lu\n", inode->data, inode->size);
    if (inode) {
        // find the lowest unused file descriptor
        proc = arch_cpu_get()->proc;
        for (fd = 0; fd < NFDS; fd++) {
            if (!proc->fds[fd]) break;
        }

        // no more fds
        if (fd == NFDS) {
            RET(tf) = -2;
            return;
        }

        extern struct file_ops inode_ramfs_ops;
        proc->fds[fd] = file_alloc(inode, &inode_ramfs_ops);
        if (!proc->fds[fd])
            RET(tf) = -3;
        else
            RET(tf) = fd;
        return;
    }

    RET(tf) = -1;
}

void
sys_close (struct intr_trap_frame *tf) {
    int fd = (int)ARG0(tf);

    // make sure fd is a valid integer [0; NFDS-1]
    if (fd < 0 || fd >= NFDS) {
        RET(tf) = -EBADF;
    } else {
        struct file* f = arch_cpu_get()->proc->fds[fd];
        if (!f) {
            RET(tf) = -EBADF;  // f isn't an opened file descriptor
        } else {
            // try closing f, then mark the file descriptor as closed for proc
            file_close(f);
            arch_cpu_get()->proc->fds[fd] = NULL;
        }
    }
}

void
sys_lseek(struct intr_trap_frame *tf) {
    int fd = (int)ARG0(tf);

    if (fd < 0 || fd >= NFDS) {
        RET(tf) = -EBADF;
    } else {
        struct file* f = arch_cpu_get()->proc->fds[fd];
        if (!f) {
            RET(tf) = -EBADF;
        } else {
            if (f->ops->lseek) {
                RET(tf) = f->ops->lseek(f, (off_t)ARG1(tf), (int)ARG2(tf));
            } else {
                RET(tf) = -EBADF;   // not seekable file
            }
        }
    }

}

/* The dup() system call allocates a new file descriptor that refers to the same
 * open file descriptor oldfd. The new file descriptor is guaranteed to be the
 * lowest-numbered file descriptor that was unused by the calling process. */
void
sys_dup(struct intr_trap_frame *tf) {
    struct process *proc = arch_cpu_get()->proc;
    int oldfd = (int)ARG0(tf), newfd = -1;

    // make sure oldfd is a valid opened file descriptor
    if (oldfd < 0 || oldfd >= NFDS)
        goto ebadf;   // invalid argument
    if (!proc->fds[oldfd])
        goto ebadf;   // oldfd is closed

    // oldfd is valid, now we find the lowest unused file descriptor
    for (int i = 0; i < NFDS; i++) {
        if (!proc->fds[i]) { newfd = i; break; };
    }

    // we found a suitable file descriptor
    if (newfd != -1) {
        proc->fds[oldfd]->refcount++;
        proc->fds[newfd] = proc->fds[oldfd];
    }
    RET(tf) = newfd;
    return;

    // == errors ==
ebadf:
    RET(tf) = -EBADF;
}

/* The dup2() system call performs the same task as dup(), but instead of
 * using the lowest-numbered unused file descriptor, it uses newfd. In other
 * words, newfd is adjousted to refer to oldfd, and the two can be used interchangeably.
 * If newfd was previously open, it is silently closed before being reused. */
void
sys_dup2(struct intr_trap_frame *tf) {
    struct process *proc = arch_cpu_get()->proc;
    int oldfd = (int)ARG0(tf), newfd = ARG1(tf);

    // make sure oldfd is valid and open
    if (oldfd < 0 || oldfd >= NFDS) goto ebadf;
    if (!proc->fds[oldfd]) goto ebadf;

    // make sure newfd is valid
    if (newfd < 0 || newfd >= NFDS) goto ebadf;

    // silently close newfd if already open and different from oldfd
    if (proc->fds[newfd] && newfd != oldfd) {
        kprintf("sys_dup2 -> closing newfd\n");
        file_close(proc->fds[newfd]);
    }

    proc->fds[oldfd]->refcount++;
    proc->fds[newfd] = proc->fds[oldfd];

    RET(tf) = newfd;
    return;

    // == errors ==
ebadf:
    RET(tf) = -EBADF;
}

void
sys_yield (struct intr_trap_frame *tf) {
  schedule();
  RET(tf) = 0;
}

void
sys_set_fg_proc (struct intr_trap_frame *tf)
{
  struct process *new_fg = proc_by_pid((pid_t)ARG0(tf));
  if (new_fg == NULL)
    RET(tf) = -1;
  else if (new_fg->pid <= 1)
    RET(tf) = -2;
  else
  {
    tty0.foreground = new_fg;
    RET(tf) = new_fg->pid;
  }
}

void
sys_sched_get_stats(struct intr_trap_frame *tf) {
    struct process *p = arch_cpu_get()->proc;
    arch_copy_to_user((void*)ARG0(tf), &p->sched_stats, sizeof(p->sched_stats));
    RET(tf) = ticks;
}

void
sys_sched_set_prio(struct intr_trap_frame *tf) {
    pid_t pid = (pid_t)ARG0(tf);
    uint64_t prio = (uint64_t)ARG1(tf);

    struct process *p = proc_by_pid(pid);
    if (p == NULL)
        RET(tf) = -EINVAL;
    else if (arch_cpu_get()->proc != p->parent)
        RET(tf) = -EPERM;
    else
        RET(tf) = (size_t)sched_set_prio(p, prio);
}
