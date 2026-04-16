/*
 *  acpi.c has the job to parse the Limine's RSDP request
 *  and populate a generalized table with the most important
 *  informations that arch-specific code can then use.
 */
#include "learnix/acpi.h"
#include "learnix/arch/amd64/types.h"
#include "learnix/lib/kpanic.h"
#include "learnix/mm/memlayout.h"
#include <learnix/lib/debug.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <learnix/types.h>
#include <limine.h>
#include <stdint.h>

__attribute__ ((
    used,
    section (".limine_requests"))) static volatile struct limine_rsdp_request
    rsdp_request = { .id = LIMINE_RSDP_REQUEST_ID, .revision = 5 };

static struct acpi_summary acpi_summary = { 0 };

static bool
xsdp_validate_checksum (struct xsdp *xsdp)
{
  unsigned char sum = 0;
  for (uint32_t i = 0; i < xsdp->length; ++i)
    sum += ((char *)xsdp)[i];
  return sum == 0;
}

static bool
xsdt_validate_checksum (struct xsdt *xsdt)
{
  unsigned char sum = 0;
  for (uint32_t i = 0; i < xsdt->hdr.length; ++i)
    sum += ((char *)xsdt)[i];
  return sum == 0;
}

static void
fadt_parse (struct fadt *fadt)
{
  if (!xsdt_validate_checksum((struct xsdt*)fadt))
    kpanic ("fadt_parse: invalid checksum");

  // 8042 PS/2 controller support (bit 1 set)
  acpi_summary.ps2_exists = fadt->iapc_boot_arch & 2;
}

static void
madt_parse (struct madt *madt)
{
  /* MADT has variable lenght entries so we can't precompute them
   * in advance like xsdt_parse() does. */
  vaddr_t curr = (vaddr_t)madt->entries, end = (vaddr_t)madt + madt->hdr.length;
  while (curr < end)
  {
    uint8_t type = *(uint8_t *)curr, length = *(uint8_t *)(curr + 1);
    switch (type)
    {
    case 1:
    {
      struct madt_entry_ioapic *io = (struct madt_entry_ioapic *)curr;
      acpi_summary.ioapic_addr = (physaddr_t)io->ioapic_addr;
      break;
    }
    case 2:
    {
      struct madt_entry_ioapic_iso *e = (struct madt_entry_ioapic_iso *)curr;
      acpi_summary.overrides[acpi_summary.overrides_len].irq_src = e->irq_src;
      acpi_summary.overrides[acpi_summary.overrides_len].flags = e->flags;
      acpi_summary.overrides[acpi_summary.overrides_len].gsi = e->gsi;
      acpi_summary.overrides_len++;
      break;
    }
    default:
      break;
    }
    curr += length;
  }
}

static void
xsdt_parse (struct xsdt *xsdt)
{
  if (!xsdt_validate_checksum (xsdt))
    kpanic ("xsdt_parse: invalid checksum");

  uint32_t entries = (xsdt->hdr.length - sizeof (struct xsdt_hdr)) / 8;
  for (uint32_t i = 0; i < entries; ++i)
  {
    // current entry's header
    struct xsdt_hdr *e = (struct xsdt_hdr *)PA_TO_HHDM (xsdt->entries[i]);

    // call the correct parser for the entry's type
    if (memcmp (e->signature, "APIC", 4) == 0)
      madt_parse ((struct madt *)e);
    else if (memcmp(e->signature, "FACP", 4) == 0)
      fadt_parse ((struct fadt*)e);
  }

  return;
}

void
acpi_init ()
{
  // we can't proceed if Limine didn't provide us the RSDP
  if (rsdp_request.response == NULL)
    kpanic ("acpi_init: RSDP response not found");

  // identify the type
  struct rsdp *rsdp = (struct rsdp *)rsdp_request.response->address;

  // TODO: validate rsdp checksum

  switch (rsdp->revision)
  {
  case 1:
    kpanic ("acpi_init: ACPI revision 1 not supported");
  case 2:
  {
    // in revision 2 we have the XSDP
    struct xsdp *xsdp = (struct xsdp *)rsdp;

    if (!xsdp_validate_checksum (xsdp))
      kpanic ("acpi_init: invalid XSDP checksum");

    // follow the XSDT pointer
    xsdt_parse ((struct xsdt *)PA_TO_HHDM (xsdp->xsdt_addr));

    break;
  }
  default:
    kpanic ("acpi_init: unknown ACPI revision %u", rsdp->revision);
  }

  kprintf ("ACPI Summary\n");
  kprintf ("I/O APIC address => %p\n", acpi_summary.ioapic_addr);
  for (int i = 0; i < acpi_summary.overrides_len; i++)
  {
    kprintf (" irq %d to gsi %d\n", acpi_summary.overrides[i].irq_src,
             acpi_summary.overrides[i].gsi);
  }
}

bool
acpi_get_ps2 (void)
{
  return acpi_summary.ps2_exists;
}

physaddr_t
acpi_get_ioapic (void)
{
  return acpi_summary.ioapic_addr;
}
