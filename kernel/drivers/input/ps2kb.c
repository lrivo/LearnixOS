#include <learnix/arch/interrupts.h>
#include <learnix/arch/io.h>
#include <learnix/drivers/console/console.h>
#include <learnix/drivers/input/ps2kb.h>
#include <learnix/lib/kpanic.h>
#include <learnix/lib/kprintf.h>
#include <stdint.h>

static int break_code = 0;

// TODO only handles basic printable chars, no control keys and no extended
// scancodes
static void
ps2_handler (struct intr_stack_frame_t *f)
{
  uint8_t scancode = pio_read8 (DATA_PORT);

  if (scancode == 0xF0)
  {
    break_code = 1;
    goto eoi;
  }

  if (scancode == 0xE0)
    goto eoi;

  if (!break_code)
    console_putchar (set2_to_ascii[scancode]);

  break_code = 0;

eoi:
  arch_interrupts_eoi (1);
}

/* Must be called before trying to read DATA_PORT. */
static void
ps2_wait_read (void)
{
  while (!(pio_read8 (CONTROL_PORT) & 0x01))
    ;
}

/* Must be called before trying to write any data. */
static void
ps2_wait_write (void)
{
  while (pio_read8 (CONTROL_PORT) & 0x02)
    ;
}

static uint8_t
ps2_read_resp (void)
{
  ps2_wait_read ();
  return pio_read8 (DATA_PORT);
}

/* PS/2 command with no argument. */
static void
ps2_cmd (uint8_t cmd)
{
  ps2_wait_write ();
  pio_write8 (CONTROL_PORT, cmd);
}

static void
ps2_kb_cmd (uint8_t cmd)
{
  ps2_wait_write ();
  pio_write8 (DATA_PORT, cmd);
}

static void
ps2_kb_cmd_data (uint8_t cmd, uint8_t data)
{
  ps2_kb_cmd (cmd);
  while (ps2_read_resp () != 0xFA)
    ;
  ps2_kb_cmd (data);
  while (ps2_read_resp () != 0xFA)
    ;
}

/* PS/2 command with argument. */
static void
ps2_cmd_data (uint8_t cmd, uint8_t data)
{
  // send the command
  ps2_cmd (cmd);
  // send the argument to the data port
  ps2_wait_write ();
  pio_write8 (DATA_PORT, data);
}

void
ps2kb_init ()
{
  // flush the output buffer
  while (pio_read8 (CONTROL_PORT) & 0x01)
    pio_read8 (DATA_PORT);
  kprintf ("ps2kb_init: output buffer flushed\n");

  // disable both ports before self tests
  ps2_cmd (0xAD);
  ps2_cmd (0xA7);

  // controller self-test
  ps2_cmd (0xAA);
  kassert_msg (0x55 == ps2_read_resp (), "PS/2 controller self-test failed");

  // test port 1
  ps2_cmd (0xAB);
  kassert_msg (0x00 == ps2_read_resp (), "PS/2 port 1 test failed");

  // test port 2
  ps2_cmd (0xA9);
  kassert_msg (0x00 == ps2_read_resp (), "PS/2 port 2 test failed");

  // modify config
  ps2_cmd (0x20);
  uint8_t config = ps2_read_resp ();

  config |= (1 << 0);  // enable IRQ1 (keyboard)
  config &= ~(1 << 6); // disable translation

  // write back the modified config
  ps2_cmd_data (0x60, config);

  // configure scancode set 2
  ps2_kb_cmd_data (0xF0, 0x02);

  // re-enable both ports
  ps2_cmd (0xAE);
  ps2_cmd (0xA8);
  
  // register the PS/2 keyboard interrupt handler
  arch_interrupts_register (0x21, ps2_handler, INTR_FLAG_DEFAULT);
  kprintf ("PS/2 keyboard registered\n");
}
