#pragma once
#include <learnix/arch/amd64/types.h>

/* A single x86_64 GDT entry */
typedef struct
{
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t flags; // only the 4 MSB
  uint8_t base_high;
} __attribute__ ((packed)) gdt_entry_t;

/* The content of the GDTR register, which informs the CPU about the installed
 * GDT */
typedef struct
{
  uint16_t size;    // total size of the GDT in bytes substracted by 1
  uintptr_t offset; // the linear address of the GDT (paging applied)
} __attribute__ ((packed)) gdtr_t;

/* Reloads CS and DS with a far jump instruction. */
extern void reloadSegments ();

/* Called by arch_stage_1() to initialize the GDT. */
void gdt_init ();
