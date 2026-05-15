#pragma once
#include <learnix/types.h>
#include <learnix/scheduler.h>

#define TTY_BUF_LEN 1024

/* the actual TTY state */
struct tty_ctx
{
  /* canonical mode buffer + state */
  char edit_buf[TTY_BUF_LEN];
  uint32_t edit_idx;	 // write index 
  uint32_t lines;        // number of canonical lines (ending in \n)

  // the wait_queue for processes that called read()
  struct wait_queue *read_q; 

  // screen driver's putchar function
  void (*putchar)(char c);
};

/* Initializes a single tty that writes on the framebuffer */
void tty_init(void (*putchar)(char c));

/* Called by the keyboard interrupt whenever a key is pressed */
void tty_line_discipline(struct tty_ctx *tty, char c);

/* Copies up to count characters into buf, called by sys_read() */
ssize_t tty_read(struct tty_ctx *tty, char *buf, size_t count);
