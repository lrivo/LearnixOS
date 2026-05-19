#include <learnix/cpu.h>
#include <learnix/lib/kprintf.h>
#include <learnix/scheduler.h>
#include <learnix/syscall.h>
#include <learnix/tty.h>

extern struct tty_ctx tty0;

void
syscall_dispatcher (struct intr_trap_frame *tf)
{
  size_t num = NUM (tf);
  switch (num)
  {
  case SYS_READ:
    sys_read (tf);
    break;
  case SYS_WRITE:
    sys_write (tf);
    break;
  case SYS_GETPID:
    sys_getpid (tf);
    break;
  case SYS_FORK:
    sys_fork (tf);
    break;
  case SYS_EXECVE:
    sys_execve (tf);
    break;
  case SYS_EXIT:
    sys_exit (tf);
    break;
  case SYS_WAIT:
    sys_wait(tf);
    break;
  case SYS_SET_FG_PROC:
    sys_set_fg_proc(tf);
    break;
  default:
    RET(tf) = -1;
    break;
  }
}

void
sys_read (struct intr_trap_frame *tf)
{
  char *buf = (char*)ARG1(tf);
  size_t count = (size_t)ARG2(tf);

  RET(tf) = tty_read(&tty0, buf, count);
}

// FIXME: this now works because i am only using it for stdout
// but it must be refactored to potentially block when I'll have pipes
void
sys_write (struct intr_trap_frame *tf)
{
  char *buf = (char *)ARG1 (tf);
  size_t count = (size_t)ARG2 (tf);

  // FIXME: not validating user pointer
  for (size_t i = 0; i < count; i++)
    kprintf ("%c", buf[i]);

  // write() should return the number of written chars
  RET(tf) = count;
}

/* for now we just mark the current process
   as zombie and immediately schedule to the next
   runnable process. */
void
sys_exit (struct intr_trap_frame *tf)
{
  struct process *proc = arch_cpu_get()->proc;  

  // now we become a zombie and wakeup the parent
  proc->state = ZOMBIE;
  wake_up(&proc->parent->child_wq);

  schedule ();
  
  // will never return here
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
