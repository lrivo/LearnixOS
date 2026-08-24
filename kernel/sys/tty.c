#include <learnix/cpu.h>
#include <learnix/tty.h>
#include <learnix/fs.h>
#include <learnix/lib/string.h>

// the global tty always in canonical mode
struct tty_ctx tty0 = { 0 };

// vtables for stdin and stdout (see main.c)
struct file_ops tty_ops_r = { 0 };
struct file_ops tty_ops_w = { 0 };

void
tty_init(void (*putchar)(char c))
{
  // TODO: tty_close(), should free the struct tty_ctx
  tty_ops_r.read = tty_read;
  tty_ops_w.write = tty_write;

  // just setup tty0's putchar function pointer
  tty0.putchar = putchar;
}

void
tty_line_discipline(struct tty_ctx *tty, char c)
{
  // newline = a full canonical line
  if (c == '\n')
  {
    tty->edit_buf[tty->edit_idx++] = '\n';
    tty->lines++;
    tty->putchar(c);	// echo to screen

    /* Wakes up all the processes sleeping on a read()
       syscall by setting their state to READY.
       So that, at the next timer interrupt, they will be run */
    wake_up(&tty->read_q);
    return;
  }

  // FIXME: screen clear
  if (c == 0x02) {
    tty->edit_idx = 0;
    tty->lines = 0;
    tty->putchar(c);
  }

  // FIXME: CTR+C should send a SIGINT signal to the foreground process
  // instead of killing it directly
  if (c == 0x03)
  {
    // avoid killig init or the shell process
    if (tty->foreground->pid > 2)
    {
      // FIXME: do a proc_exit() function that both sys_exit and this
      // can call to safely update state
      tty->foreground->state = ZOMBIE;
      sched_dequeue(tty->foreground);
      if (tty->foreground->parent != NULL)
        wake_up(&tty->foreground->parent->child_wq);
      arch_cpu_get()->proc_need_resched = true;
    }

    // discard everything
    tty->edit_idx = 0;
    tty->lines = 0;
    return;
  }

  /* Echo printable chars on screen (to see what you type) */
  if (tty->edit_idx < TTY_BUF_LEN - 1)
  {
    if (c >= 32 && c < 127)
    {
      tty->edit_buf[tty->edit_idx++] = c;
      tty->putchar(c);
    }
  }
}

ssize_t
tty_read(struct file *f, void *buf, size_t count)
{
  struct tty_ctx *tty = (struct tty_ctx*)f->ptr;

  /* wait untill we are the foreground process and
   * a canonical line is ready. */
  while (tty->lines == 0)
    sleep_on(&tty->read_q);

  // how much bytes we can copy?
  int n = (tty->edit_idx < count) ? tty->edit_idx : count;

  // copy into buf (sys_read has already validated it)
  memcpy((void*)buf, (void*)tty->edit_buf, n);

  // mark the line as consumed
  tty->edit_idx = 0;
  tty->lines = 0;

  return n;
}

ssize_t
tty_write(struct file *f, void *buf, size_t count)
{
  struct tty_ctx *tty = (struct tty_ctx*)f->ptr;
  const char *buff = (char*)buf;

  for (size_t i = 0; i < count; i++)
    tty->putchar(buff[i]);

  return count;
}
