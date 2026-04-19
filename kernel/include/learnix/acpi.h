#pragma once
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
  bool ps2_exists;     // does the PS/2 controller exists?
  paddr_t ioapic_addr; // MMIO physical address of the I/O APIC

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
// FADT (Fixed ACPI Description Table)
// https://uefi.org/sites/default/files/resources/ACPI_Spec_6_5_Aug29.pdf
//===========
struct fadt
{
  struct xsdt_hdr hdr;
  uint32_t firmware_ctrl;
  uint32_t dsdt;
  uint8_t reserved;
  uint8_t preferred_pm_profile;
  uint16_t sci_int;
  uint32_t smi_cmd;
  uint8_t acpi_enable;
  uint8_t acpi_disable;
  uint8_t s4bios_req;
  uint8_t pstate_cnt;
  uint32_t pm1a_evt_blk;
  uint32_t pm1b_evt_blk;
  uint32_t pm1a_cnt_blk;
  uint32_t pm1b_cnt_blk;
  uint32_t pm2_cnt_blk;
  uint32_t pm_tmr_blk;
  uint32_t gpe0_blk;
  uint32_t gpe1_blk;
  uint8_t pm1_evt_len;
  uint8_t pm1_cnt_len;
  uint8_t pm2_cnt_len;
  uint8_t pm_tmr_len;
  uint8_t gpe0_blk_len;
  uint8_t gpe1_blk_len;
  uint8_t gpe1_base;
  uint8_t cst_cnt;
  uint16_t p_lv2_lat;
  uint16_t p_lvl3_lat;
  uint16_t flush_size;
  uint16_t flush_stride;
  uint8_t duty_offset;
  uint8_t duty_width;
  uint8_t day_alarm;
  uint8_t mon_alarm;
  uint8_t century;
  uint16_t iapc_boot_arch; // bit 1 set = PS/2 controller exists
  // TODO: FADT has many more field that I currently don't care about
} __attribute__ ((packed));

//===========
// MADT
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

/* Returns true if the motherboard supports PS/2 emulation. */
bool acpi_get_ps2 (void);

/* Returns the I/O APIC physical address found in MADT. */
paddr_t acpi_get_ioapic ();
