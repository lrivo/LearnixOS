#include <learnix/lib/rand.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>

/*
 * This is a very bad PRNG described here:
 * http://wiki.osdev.org/Random_Number_Generator#Rolling_your_own
 *
 * Intended as a simple example for implementing your own PRNG for
 * LearnixOS.
 */

static uint64_t seed;

void
rand_init()
{
  // request 8 random bytes from the hardware
  arch_rand_bytes((uint8_t*)&seed, 8);
}

int
rand_bytes(uint8_t* vec, size_t n)
{
  /* Since we only have a single 64 bit seed, we can
   * safely copy only 8 bytes at a time. */
  size_t can_copy = 8;
  while (n > 0)
  {
    // transform the seed a bit 
    seed = (seed * 0x4B4B9656U) ^ (seed * 0x565AC3C3U) + 1;
    
    // can we copy 8 bytes?
    if (n >= 8)
      can_copy = 8;
    else
      can_copy = n;

    memcpy(vec, &seed, can_copy);
    vec += can_copy;
    n -= can_copy;
  }

  return n;
}
