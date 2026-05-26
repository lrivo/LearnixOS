#pragma once
#include <learnix/types.h>

/* Asks the hardware for n bytes of random numbers and
 * writes them starting from vec. */
int arch_rand_bytes(void* vec, size_t n);

/* Seeds the random number generator using arch_rand_bytes()
 * to get truly random bytes from the HW. */
void rand_init();

/* Asks the PRNG backend (NOT the hardware) for n random bytes
 * and writes them at vec. */ 
void rand_bytes(void* vec, size_t n);
