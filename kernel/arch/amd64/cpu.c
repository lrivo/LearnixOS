#include "amd64.h"
#include "gdt.h"
#include "idt.h"
#include "ioapic.h"
#include "lapic.h"
#include <learnix/cpu.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
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
}

void
arch_stage_2 ()
{
  lapic_init ();
  ioapic_init ();

  // setup per-core struct
  struct cpu *cpu = (struct cpu *)kzalloc (sizeof (struct cpu));
  cpu->self = cpu;
  cpu->proc_need_resched = true;
  wrmsr (0xC0000101, (uint64_t)cpu);
  wrmsr (0xC0000102, (uint64_t)cpu);
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
