#include <learnix/lib/rand.h>
#include <learnix/lib/kprintf.h>
#include <learnix/security/canary.h>

void
canary_generate(vaddr_t at)
{
  // set MSB to 0 an generate 7 random bytes
  uint64_t canary = 0;
  rand_bytes(&canary, 7);
  kprintf("[INFO] canary 0x%p\n", canary);
  
  // copy the canary where the caller requested 
  *((uint64_t*)at) = canary;
}
