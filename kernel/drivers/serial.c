#include <learnix/io.h>
#include <learnix/drivers/serial.h>

int
serial_init(void)
{
  // disable interrupts.
  pio_write8(COM1_INTERRUPT_ENABLE_REGISTER, 0x00);
  
  // enable DLAB register.
  pio_write8(COM1_DLAB_REGISTER, 0x80);

  // set divisor to 3 (3400 baud).
  pio_write8(COM1_DIVISOR_LSB_REGISTER, 0x03);
  pio_write8(COM1_DIVISOR_MSB_REGISTER, 0x00);

  // 8 bits, no parity, 1 stop bit.
  pio_write8(COM1_DLAB_REGISTER, 0x03);

  // set FIFO, clear transmit, clear receive.
  pio_write8(COM1_FIFO_CONTROL_REGISTER, 0xC7);

  // set RTS/DSR and enable IRQs.
  pio_write8(COM1_MODEM_CONTROL_REGISTER, 0x0B);

  // enble loopback mode.
  pio_write8(COM1_MODEM_CONTROL_REGISTER, 0x1E);

  // test
  pio_write8(COM1, 0xAE);
  if (pio_read8(COM1) != 0xAE)
    return -1;

  // if COM1 is working set it in normal operation mode
  // disable loopback, IRQs enabled, OUT#1 and OUT#2 bits set
  pio_write8(COM1_MODEM_CONTROL_REGISTER, 0x0F);

  return 0;
}

void
serial_putchar(const char c)
{
  // poll untill the TEMT (Transmitter Empty) bit is set. 
  while ((pio_read8(COM1_LINE_STATUS_REGISTER) & 0x20) == 0)
    ;
  
  // write c to COM1's port
  pio_write8(COM1, (uint8_t)c);
}
