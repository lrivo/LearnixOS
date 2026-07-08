#include "learnix/arch/types.h"
#include "learnix/lib/elf.h"
#include "learnix/lib/limine_module.h"
#include <learnix/lib/rand.h>
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
#include <learnix/tty.h>
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
 
  // Initialize the TTY 	
  tty_init(console_putchar);

  // Initialize the virtual memory manager
  vmm_init ();

  // Essential CPU initialization like exception handlers
  arch_stage_1 ();

  // Seed the (C)SPRNG
  rand_init();

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
  
  // Initialize the init process (PID 1)
  extern struct file tty_file;
  //proc_by_pid(0)->fds[0] = tty_file;
  //proc_by_pid(0)->fds[1] = tty_file;

  struct process *init = proc_create();
  init->fds[0] = tty_file; 
  init->fds[1] = tty_file;
  struct elf64_hdr *elf = (struct elf64_hdr*)limine_module_get("/boot/init");
  elf_load(elf, init->pgtable);
  arch_proc_init(init, elf->e_entry, USR_STACK + PGSIZE);
  sched_enqueue(init);
  
  // Done, the kernel idle process will spin here forever
  arch_interrupts_enable ();
  arch_cpu_hcf ();
}
