#include <learnix/arch/arch.h>
#include <learnix/arch/interrupts.h>
#include <learnix/drivers/console/console.h>
#include <learnix/drivers/input/ps2kb.h>
#include <learnix/lib/debug.h>
#include <learnix/lib/kpanic.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <learnix/mm/memlayout.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/types.h>
#include <limine.h>

// Set the base revision to 5, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__ ((used, section (".limine_requests"))) static volatile uint64_t
    limine_base_revision[] = LIMINE_BASE_REVISION (5);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__ ((
    used,
    section (
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request
    = { .id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0 };

// Limine memory map
__attribute__ ((
    used,
    section (".limine_requests"))) static volatile struct limine_memmap_request
    memmap_request = { .id = LIMINE_MEMMAP_REQUEST_ID, .revision = 4 };

// HHDM (Higher Half Direct Mapping)
__attribute__ ((
    used,
    section (".limine_requests"))) static volatile struct limine_hhdm_request
    hhdm_request = { .id = LIMINE_HHDM_REQUEST_ID, .revision = 4 };

// Kernel executable load addresses
__attribute ((used, section (".limine_requests"))) static volatile struct
    limine_executable_address_request exec_request
    = { .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID, .revision = 4 };

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__ ((used,
                section (".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__ ((used,
                section (".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// memlayout.h global variables needed for translations
uintptr_t hhdm_offset;
uintptr_t kernel_virt_base;
uintptr_t kernel_phys_base;

// Tell kprintf() to use the framebuffer console to print stuff.
void
_putchar (char character)
{
  console_putchar (character);
}

/* Kernel's entrypoint function as defined by the linker script.
 * Has the job to initialize all subsystems as then hand the CPU
 * to the scheduler. */
void
kmain (void)
{
  arch_interrupts_disable ();

  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED (limine_base_revision) == false)
  {
    arch_hcf ();
  }

  // save the HDDM base address for the global translation macros.
  hhdm_offset = hhdm_request.response->offset;
  kernel_virt_base = exec_request.response->virtual_base;
  kernel_phys_base = exec_request.response->physical_base;

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL
      || framebuffer_request.response->framebuffer_count < 1)
  {
    arch_hcf ();
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer
      = framebuffer_request.response->framebuffers[0];

  // Initialize the frambuffer console,
  struct console_fb_info fb_info = { framebuffer->address, framebuffer->width,
                                     framebuffer->height, framebuffer->pitch };
  console_init (fb_info);

  // Minimal CPU intialization, basic interrupts and exception handlers.
  arch_stage_1 ();

  // Initialize the physical memory allocator using Limine's memmap.
  pmm_init (memmap_request.response);

  // Initialize the PS/2 keyboard.
  ps2kb_init ();

  // test
  vaddr_t kern_pgtable = vmm_get_pgtable ();
  // get a zero-ed physical frame
  physaddr_t pg = pmm_alloc (PMM_ZERO);
  kprintf ("pmm_alloc => 0x%lx\n", pg);
  // virtually map it
  if (vmm_map ((void *)kern_pgtable, 4096, pg, 0))
  {
    kprintf ("page mapped!\n");
    memset ((void *)4096, 0x41, 4096);
    dbg_hexdump ((void *)4096, 4);
  }
  vaddr_t va2 = 2 * PGSIZE;
  vmm_map ((void *)kern_pgtable, va2, pg, 0);
  memset ((void *)va2, 0x42, 8);
  dbg_hexdump ((void *)4096, 4);

  // We're done, enable interrupts and hang this core
  arch_interrupts_enable ();

  arch_hcf ();
}
