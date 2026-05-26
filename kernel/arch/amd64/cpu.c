#include "amd64.h"
#include "gdt.h"
#include "idt.h"
#include "ioapic.h"
#include "lapic.h"
#include <learnix/cpu.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <learnix/lib/rand.h>
#include <learnix/mm/kmalloc.h>
#include <stdbool.h>
#include <stdint.h>

static inline void
cpuid (uint32_t code, uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
       uint32_t *edx)
{
  asm volatile ("cpuid"
                : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                : "a"(code), "c"(0)
                : "memory");
}

void
arch_stage_1 ()
{
  gdt_init ();
  idt_init ();
  
  // enable the NX bit
  wrmsr (0xC0000080, rdmsr(0xC0000080) | (1ULL << 11));
}

void
arch_stage_2 ()
{
  lapic_init ();
  ioapic_init ();
  
  // FIXME: setup FS at this address for every process
  wrmsr (0xC0000100, (uint64_t)0x300000);

  // setup per-core struct
  struct cpu *cpu = (struct cpu *)kzalloc (sizeof (struct cpu));
  cpu->self = cpu;
  wrmsr (0xC0000101, (uint64_t)cpu);
  wrmsr (0xC0000102, (uint64_t)cpu);

  // enable SYSCALL/SYSRET
  uint64_t efer = rdmsr (0xC0000080) | (1 << 0);
  wrmsr (0xC0000080, efer);
  // configure the STAR register
  uint64_t star
      = ((uint64_t)0x0010 << 48) | ((uint64_t)0x0008 << 32) | (uint32_t)0;
  wrmsr (0xC0000081, star);
  // configure the syscall entrypoint function in LSTAR
  extern void syscall_entry (void);
  wrmsr (0xC0000082, (uint64_t)syscall_entry); // TODO:
  // disable interrupts while serving a syscall
  wrmsr (0xC0000084, (1 << 9));
}

struct cpu *
arch_cpu_get (void)
{
  struct cpu *p;
  asm volatile ("mov %%gs:0, %0" : "=r"(p));
  return p;
}

void
arch_cpu_identify (struct cpu_info *c)
{
  uint32_t eax, ebx, ecx, edx, unused;

  // zero-out the struct before filling it
  memset ((void *)c, 0x00, sizeof (struct cpu_info));

  // 0x8000_0001 Extended Processor Feature Identifier
  char *n = c->name;
  for (uint32_t leaf = 0x80000002; leaf <= 0x80000004; n += 4, leaf++)
  {
    cpuid (leaf, &eax, &ebx, &ecx, &edx);
    memcpy (n, &eax, 4);
    memcpy (n += 4, &ebx, 4);
    memcpy (n += 4, &ecx, 4);
    memcpy (n += 4, &edx, 4);
  }

  // 0x8000_0008: Processor Capacity Parameters and Extended Feature
  // Identification
  cpuid (0x80000008, &eax, &unused, &unused, &unused);
  c->pa_bits_max = eax & 0xFF;        // 7:0
  c->va_bits_max = (eax >> 8) & 0xFF; // 15:8
}

void
arch_cpu_hcf ()
{
  for (;;)
  {
    asm volatile ("hlt");
  }
}

extern int rdseed_u64(uint64_t* out);

int
arch_rand_bytes(void *vec, size_t n)
{
  uint8_t *v = (uint8_t*)vec;
  uint64_t rdseed, can_copy;
  
  while (n > 0)
  {
    // generate a random number
    if (rdseed_u64(&rdseed) < 0)
      return -1;

    // how many bytes can we copy?
    can_copy = n < 8 ? n : 8;

    memcpy(v, &rdseed, can_copy);
    v += can_copy;
    n -= can_copy;
  }
  return 0;
}
