#pragma once
#include <learnix/types.h>

/* Asks the hardware for n bytes of random numbers and
 * writes them starting from vec. */
void arch_rand_bytes(uint8_t* vec, size_t n);

/* Seeds the random number generator using arch_rand_bytes()
 * to get truly random bytes from the HW. */
void rand_init();

/* Asks the PRNG backend (NOT the hardware) for n random bytes
 * and writes them at vec. */ 
int rand_bytes(uint8_t* vec, size_t n);
