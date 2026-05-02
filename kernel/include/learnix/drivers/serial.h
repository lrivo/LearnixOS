#pragma once

#define COM1 0x3F8
#define COM1_INTERRUPT_ENABLE_REGISTER (COM1 + 1)
#define COM1_DIVISOR_LSB_REGISTER (COM1)
#define COM1_DIVISOR_MSB_REGISTER (COM1 + 1)
#define COM1_INTERRUPT_VERIFICATION_REGISTER (COM1 + 2)
#define COM1_FIFO_CONTROL_REGISTER (COM1 + 2)
#define COM1_DLAB_REGISTER (COM1 + 3)
#define COM1_MODEM_CONTROL_REGISTER (COM1 + 4)
#define COM1_LINE_STATUS_REGISTER (COM1 + 5)        // useful to check for errors and enable polling
#define COM1_MODEM_STATUS_REGISTER (COM1 + 6)
#define COM1_SCRATCH_REGISTER (COM1 + 7)

/* Returns 0 if the COM1 serial port was initialized, otherwise a negative number. */
int serial_init();

/* Writes a byte on the serial console. Called by kprintf(). */
void serial_putchar(const char c);
