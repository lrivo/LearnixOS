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
  [SYS_MMAP] = sys_mmap,
  [SYS_MUNMAP] = sys_munmap,
  [SYS_PIPE] = sys_pipe,
  [SYS_GETPID] = sys_getpid,
  [SYS_FORK] = sys_fork,
  [SYS_EXECVE] = sys_execve,
  [SYS_EXIT] = sys_exit,
  [SYS_WAIT] = sys_wait,
  [SYS_SET_FG_PROC] = sys_set_fg_proc
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
  void *buf = (void*)ARG1(tf);
  size_t count = (size_t)ARG2(tf);
  
  struct file *f = arch_cpu_get()->proc->fds[fd];
  RET(tf) = f->ops->read(f, buf, count);
}

void
sys_write (struct intr_trap_frame *tf)
{
  int fd = (int)ARG0(tf);
  void *buf = (void*)ARG1 (tf);
  size_t count = (size_t)ARG2 (tf);
  
  // write() should return the number of written chars
  struct file *f = arch_cpu_get()->proc->fds[fd];
  RET(tf) = f->ops->write(f, buf, count);
}

void
sys_set_fg_proc (struct intr_trap_frame *tf)
{
  struct process *new_fg = proc_by_pid((pid_t)ARG0(tf));
  if (new_fg == NULL)
  {
    RET(tf) = -1;
  }
  else if (new_fg->pid <= 1)
  {
    RET(tf) = -2;
  }
  else
  {
    tty0.foreground = new_fg;
    RET(tf) = new_fg->pid;
  }
}
