#pragma once
#include "learnix/arch/amd64/types.h"
#include <learnix/types.h>

/* The output of apic_init(), it contains the most important
 * information found during ACPI parsing that the rest of
 * the kernel can use to initialize the hardware. */
struct acpi_intr_override
{
  uint8_t irq_src;
  uint16_t flags;
  uint32_t gsi;
};

struct acpi_summary
{
  physaddr_t ioapic_addr; // MMIO physical address of the I/O APIC

  struct acpi_intr_override overrides[16];
  int overrides_len;
};

/* RSDP revision 0 (now deprecated). */
struct rsdp
{
  char signature[8];
  uint8_t checksum;
  char oem_id[6];
  uint8_t revision;
  uint32_t rsdt_addr;
} __attribute__ ((packed));

/* RSDP revision 1 now uses XSDT. */
struct xsdp
{
  struct rsdp rsdp; // legacy fields
  uint32_t length;
  uint64_t xsdt_addr;
  uint8_t checksum_ext;
  uint8_t reserved[3];
} __attribute__ ((packed));

//===========
// XSDT
//===========
/* XSDT table header */
struct xsdt_hdr
{
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__ ((packed));

/* Actual XSDT table, header plus an array of entries. */
struct xsdt
{
  struct xsdt_hdr hdr;
  uint64_t entries[];
} __attribute__ ((packed));

//===========
// XSDT
//===========
struct madt
{
  struct xsdt_hdr hdr;
  uint32_t lapic_addr;
  uint32_t flags;
  uint8_t entries[];
} __attribute__ ((packed));

/* Entry type 0: logical processor LAPIC */
struct madt_entry_lapic
{
  uint8_t type;
  uint8_t len;
  uint8_t cpu_id;
  uint8_t lapic_id;
  uint32_t flags;
} __attribute__ ((packed));

/* Entry type 1: I/O APIC */
struct madt_entry_ioapic
{
  uint8_t type;
  uint8_t len;
  uint8_t id; // I/O APIC's ID
  uint8_t reserved;
  uint32_t ioapic_addr; // physical MMIO address
  uint32_t gsib;        // first physical IRQ line served by this I/O APIC
} __attribute__ ((packed));

/* Entry type 2: I/O APIC Interrupt Source Override */
struct madt_entry_ioapic_iso
{
  uint8_t type;
  uint8_t len;
  uint8_t bus_src;
  uint8_t irq_src; // IRQ line overridden
  uint32_t gsi;    // remapped global interrupt line
  uint16_t flags;
} __attribute__ ((packed));

/* Entry type 3: I/O APIC NMI */
struct madt_entry_ioapic_nmi
{
  uint8_t type;
  uint8_t len;
  uint8_t nmi_src;
  uint8_t reserved;
  uint16_t flags;
  uint32_t gsi;
} __attribute__ ((packed));

/* ACPI parsing entrypoint called early by kmain(). */
void acpi_init ();

/* Returns the I/O APIC physical address found in MADT. */
physaddr_t acpi_get_ioapic ();
