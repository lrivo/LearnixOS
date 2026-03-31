#include "gdt.h"
#include <learnix/lib/string.h>
#include <stdint.h>

// TODO: missing TSS segment (unimplemented rn)
static gdt_entry_t gdt[5];
static gdtr_t gdtr;

static inline void
gdt_load ()
{
  gdtr.size = (5 * sizeof (gdt_entry_t)) - 1;
  gdtr.offset = (uintptr_t)&gdt;

  asm volatile ("lgdt (%0)" : : "r"(&gdtr));

  reloadSegments ();
}

// NOTE: on x86_64 base and limits are meaningless, only access and flags count
void
gdt_init ()
{
  // 0x00: Null descriptor
  memset (gdt, 0, sizeof (gdt_entry_t));

  // 0x08: Kernel Mode Code Segment
  gdt[1].limit = 0;
  gdt[1].base_low = 0;
  gdt[1].base_mid = 0;
  gdt[1].base_high = 0;
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
  gdt[1].access = 0x9A;
  // NOTE: I was setting this to 0xA which was setting the L bit to 0
  // effectively marking this segment as a legacy 32-bit one and causing
  // the CPU to ignore the higher 32 bits of the jump address (thanks Gemini
  // Pro)
  gdt[1].flags = 0xA0;

  // 0x010: Kernel Mode Data Segment
  gdt[2].limit = 0;
  gdt[2].base_low = 0;
  gdt[2].base_mid = 0;
  gdt[2].base_high = 0;
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
  gdt[2].access = 0x92;
  gdt[2].flags = 0xC0;

  // 0x18: User Mode Data Segment
  gdt[3].limit = 0;
  gdt[3].base_low = 0;
  gdt[3].base_mid = 0;
  gdt[3].base_high = 0;
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
  gdt[3].access = 0xF2;
  gdt[3].flags = 0xC0;

  // 0x20: User Mode Code Segment
  gdt[4].limit = 0;
  gdt[4].base_low = 0;
  gdt[4].base_mid = 0;
  gdt[4].base_high = 0;
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
  gdt[4].access = 0xFA;
  gdt[4].flags = 0xA0;

  gdt_load ();
}
