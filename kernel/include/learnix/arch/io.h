#pragma once
#include <learnix/types.h>

inline uint8_t
pio_read8 (uint16_t port)
{
  uint8_t data;
#ifdef __x86_64__
  asm volatile ("inb %w1,%0" : "=a"(data) : "Nd"(port));
#endif
  return data;
}

inline void
pio_write8 (uint16_t port, uint8_t data)
{
#ifdef __x86_64__
  asm volatile ("outb %0,%w1" : : "a"(data), "Nd"(port));
#endif
}
