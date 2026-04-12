#pragma once
#include <learnix/arch/amd64/types.h>

/*
 * single GDT entry
 * https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
 */
struct gdt_entry
{
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t flags; // only the 4 MSB
  uint8_t base_high;
} __attribute__ ((packed));

/*
 * long mode TSS segment descriptor for the GDT
 */
struct tss_descriptor
{
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t flags; // only the 4 MSB
  uint8_t base_high;
  uint32_t base_upper; 
  uint32_t reserved;
} __attribute__ ((packed));

// https://wiki.osdev.org/Task_State_Segment#Long_Mode
struct tss_entry 
{
  uint32_t reserved0;
  uint64_t rsp0;        // kernel stack for ring 0 (ring3->ring0 interrupts)
  uint64_t rsp1;        // kernel stack for ring 1 (unused)
  uint64_t rsp2;        // kernel stack for ring 2 (unused)
  uint64_t reserved1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iopb;      // set to sizeof(struct tss) to deny any ring3 I/O port
} __attribute__((packed));

// the actual GDT
struct gdt
{
  struct gdt_entry null;
  #define GDT_KCODE 0x8
  struct gdt_entry kcode;
  #define GDT_KDATA 0x10
  struct gdt_entry kdata;
  #define GDT_UDATA 0x18
  struct gdt_entry udata; // udata and ucode must be switched for STAR
  #define GDT_UCODE 0x20
  struct gdt_entry ucode;
  #define GDT_TSS 0x28
  struct tss_descriptor tss;
} __attribute__ ((packed));

// the actual TSS (one entry for each CPU in the future)
struct tss
{
  struct tss_entry tss0;
} __attribute__ ((packed));

// the GDTR register tells the CPU where to look for the GDT
struct gdtr
{
  uint16_t size;    // total size of the GDT in bytes substracted by 1
  uintptr_t offset; // the linear address of the GDT (paging applied)
} __attribute ((packed));

/* Reloads CS and DS with a far jump instruction. */
extern void reloadSegments ();

/* Called by arch_stage_1() to initialize the GDT. */
void gdt_init ();
