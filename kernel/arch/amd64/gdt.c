#include "gdt.h"
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <stdint.h>

static struct gdt gdt;
static struct tss tss;
static struct gdtr gdtr;

// FIXME: just a test kernel stack
static char kern_stack[4096];

static inline void
gdt_load ()
{
  gdtr.size = sizeof (struct gdt) - 1;
  gdtr.offset = (uintptr_t)&gdt;
  asm volatile ("lgdt (%0)" : : "r"(&gdtr));
  reloadSegments ();
}

static inline void
tss_load ()
{
  asm volatile ("ltr %0" ::"r"((uint16_t)0x28));
}

// NOTE: on x86_64 base and limits are meaningless, only access and flags count
void
gdt_init ()
{
  memset (kern_stack, 0, 4096);
  kprintf ("gdt_init => kernel stack at %p\n", kern_stack);

  // 0x00: Null descriptor
  memset (&gdt, 0, sizeof (struct gdt_entry));

  // 0x08: Kernel Mode Code Segment
  gdt.kcode.limit = 0;
  gdt.kcode.base_low = 0;
  gdt.kcode.base_mid = 0;
  gdt.kcode.base_high = 0;
  /*
   * 0x9A -> 1001 1010
   * P = 1    (present bit)
   * DPL = 00 (kernel privilege level, aka ring 0)
   * S = 1    (descriptor type, 1 means code or data)
   * E = 1    (this is a code segment)
   * DC = 0   (this segment is only executable at ring 0)
   * RW = 1   (read/write bit, allows read access to the code segment)
   * A =  0   TODO: maybe this should be set to 1
   */
  gdt.kcode.access = 0x9A;
  // NOTE: I was setting this to 0xA which was setting the L bit to 0
  // effectively marking this segment as a legacy 32-bit one and causing
  // the CPU to ignore the higher 32 bits of the jump address (thanks Gemini
  // Pro)
  gdt.kcode.flags = 0xA0;

  // 0x010: Kernel Mode Data Segment
  gdt.kdata.limit = 0;
  gdt.kdata.base_low = 0;
  gdt.kdata.base_mid = 0;
  gdt.kdata.base_high = 0;
  /*
   * 0x92 -> 1001 0010
   * P = 1    (present bit)
   * DPL = 00 (ring 0: kernel mode privilege level)
   * S = 1    (descriptor type, 1 means code or data)
   * E = 0    (a data segment is not executable)
   * DC = 0   (this segment grows upwards)
   * RW = 1   (write access is allowed)
   * A =  0
   */
  gdt.kdata.access = 0x92;
  gdt.kdata.flags = 0xC0;

  // 0x20: User Mode Code Segment
  gdt.ucode.limit = 0;
  gdt.ucode.base_low = 0;
  gdt.ucode.base_mid = 0;
  gdt.ucode.base_high = 0;
  /*
   * 0xFA -> 1111 1010
   * P = 1    (present)
   * DPL = 3  (ring 3: usermode privilege level)
   * S = 1    (descriptor type, 1 means code or data)
   * E = 1    (this is a code segment)
   * DC = 0
   * RW = 1
   * A = 0
   */
  gdt.ucode.access = 0xFA;
  gdt.ucode.flags = 0xA0;

  // 0x18: User Mode Data Segment
  gdt.udata.limit = 0;
  gdt.udata.base_low = 0;
  gdt.udata.base_mid = 0;
  gdt.udata.base_high = 0;
  /*
   * 0xF2 -> 1111 0010
   * P = 1    (present)
   * DPL = 3  (ring 3: usermode privilege level)
   * S = 1    (descriptor type, 1 means code or data)
   * E = 0    (a data segment is not executable)
   * DC = 0
   * RW = 1
   * A = 0
   */
  gdt.udata.access = 0xF2;
  gdt.udata.flags = 0xC0;

  // 0x28: TSS
  vaddr_t tss_addr = (vaddr_t)&tss;
  gdt.tss.limit = (uint16_t)sizeof (struct tss);
  gdt.tss.base_low = tss_addr & 0xFFFF;
  gdt.tss.base_mid = (tss_addr >> 16) & 0xFF;
  gdt.tss.base_high = (tss_addr >> 24) & 0xFF;
  gdt.tss.base_upper = (tss_addr >> 32) & 0xFFFFFFFF;
  gdt.tss.access = 0x89;
  gdt.tss.flags = 0x40;

  memset (&tss.tss0, 0, sizeof (struct tss_entry));
  tss.tss0.rsp0 = (vaddr_t)kern_stack + 4096;
  tss.tss0.iopb = (uint16_t)sizeof (struct tss_entry);

  gdt_load ();
  tss_load ();
}
