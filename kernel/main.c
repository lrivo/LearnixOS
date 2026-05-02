#include "learnix/arch/types.h"
#include "learnix/scheduler.h"
#include <learnix/acpi.h>
#include <learnix/arch/memlayout.h>
#include <learnix/cpu.h>
#include <learnix/drivers/console/console.h>
#include <learnix/drivers/input/ps2kb.h>
#include <learnix/drivers/serial.h>
#include <learnix/interrupts.h>
#include <learnix/lib/kpanic.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/process.h>
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
    memmap_request = { .id = LIMINE_MEMMAP_REQUEST_ID, .revision = 5 };

// HHDM (Higher Half Direct Mapping)
__attribute__ ((
    used,
    section (".limine_requests"))) static volatile struct limine_hhdm_request
    hhdm_request = { .id = LIMINE_HHDM_REQUEST_ID, .revision = 5 };

// Kernel executable load addresses
__attribute ((used, section (".limine_requests"))) static volatile struct
    limine_executable_address_request exec_request
    = { .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID, .revision = 5 };

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__ ((used,
                section (".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__ ((used,
                section (".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void
test_proc_init (vaddr_t ucode, vaddr_t ustack, vaddr_t exec)
{
  struct process *p = proc_create ();

  // copy the exec into the userspace page
  paddr_t ucode_pf = pmm_alloc (PMM_NONE);
  memcpy ((void *)P2V (ucode_pf), (void *)exec, 64);

  // map user code as read-only and executable
  vmm_map (p->pgtable, ucode, ucode_pf, VMM_FLAG_USER | VMM_FLAG_EXEC);

  // map user stack as RW and non executable
  vmm_map (p->pgtable, ustack, pmm_alloc (PMM_ZERO),
           VMM_FLAG_USER | VMM_FLAG_WRITE);

  // initialize the process's trap frame
  arch_proc_init (p, ucode, ustack + PGSIZE);

  // add the process to the runqueue
  sched_enqueue (p);
}

// memlayout.h global variables needed for translations
vaddr_t hhdm_offset, kernel_virt_base;
paddr_t kernel_phys_base;

void
_putchar (char c)
{
  serial_putchar (c);
  console_putchar (c);
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
    arch_cpu_hcf ();
  }

  // save the HDDM base address for the global translation macros.
  hhdm_offset = hhdm_request.response->offset;
  kernel_virt_base = exec_request.response->virtual_base;
  kernel_phys_base = exec_request.response->physical_base;

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL
      || framebuffer_request.response->framebuffer_count < 1)
  {
    arch_cpu_hcf ();
  }

  // Initialize the serial console.
  serial_init ();

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer
      = framebuffer_request.response->framebuffers[0];

  // Initialize the frambuffer console,
  struct console_fb_info fb_info = { framebuffer->address, framebuffer->width,
                                     framebuffer->height, framebuffer->pitch };
  console_init (fb_info);

  vmm_init ();

  // Essential CPU initialization like exception handlers
  arch_stage_1 ();

  // ACPI parsing
  acpi_init ();

  // Initialize the physical memory allocator using Limine's memmap.
  pmm_init (memmap_request.response);

  // Initialize the kernel heap
  kmalloc_init ((vaddr_t)KMALLOC_START, PGSIZE);

  // post ACPI initialization, for running processes
  arch_stage_2 ();

  // Initialize the kernel idle process
  proc_init ();

  // Initialize the scheduler
  sched_init ();

  // Initialize the PS/2 keyboard
  ps2kb_init ();

  // TEST: idle process (PID 1)
  extern void test_ucode (void);
  extern void test_ucode_sys (void);
  vaddr_t ucode = 0x400000UL;        // 4MB
  vaddr_t ustack = 0x7ffffffdd000UL; // bottom of the user stack
  test_proc_init (ucode, ustack, (vaddr_t)test_ucode_sys);

  for (int i = 0; i < 1; i++)
    test_proc_init (ucode, ustack, (vaddr_t)test_ucode);

  // Done, the kernel idle process will spin here forever
  arch_interrupts_enable ();
  arch_cpu_hcf ();
}
