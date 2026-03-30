#include "lib/kprintf.h"
#include "mm/pmm.h"
#include <mm/memlayout.h>
#include <drivers/console.h>
#include <interrupts.h>
#include <drivers/input/ps2kb.h>
#include <core.h>
#include <arch.h>
#include <lib/string.h>
#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Set the base revision to 5, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests"))) static volatile uint64_t
    limine_base_revision[] = LIMINE_BASE_REVISION(5);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

// Limine memory map
__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_memmap_request
    memmap_request = {.id = LIMINE_MEMMAP_REQUEST_ID, .revision = 4};

// HHDM
__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_hhdm_request
    hhdm_request = {.id = LIMINE_HHDM_REQUEST_ID, .revision = 4};

// Kernel executable load addresses
__attribute((
  used,
  section(".limine_requests"))) static volatile struct limine_executable_address_request
  exec_request = {.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID, .revision = 4};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used,
               section(".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

/* GLOBAL VARIABLES */
uintptr_t hhdm_offset = 0;
uintptr_t kernel_virt_base = 0;
uintptr_t kernel_phys_base = 0;

/* kprintf() needs to know how to print characters, we route him to our framebuffer console. */
void _putchar(char character) {
  console_putchar(character);
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
  // disable interrupts
  arch_interrupts_disable();

  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
    arch_hcf();
  }
  
  // save the HDDM base address for the global translation macros 
  hhdm_offset = hhdm_request.response->offset;
  kernel_virt_base = exec_request.response->virtual_base;
  kernel_phys_base = exec_request.response->physical_base;

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
    arch_hcf();
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer =
      framebuffer_request.response->framebuffers[0];
  
  // Initialize the frambuffer console
  struct console_fb_info fb_info = {
    framebuffer->address,
    framebuffer->width,
    framebuffer->height,
    framebuffer->pitch
  };
  console_init(fb_info);
  
  // Initialize the CPU
  arch_stage_1();
	
  // Initialize the Physical Memory Manager configured at compile-time
  pmm_init(memmap_request.response);

  // Initialize the PS/2 keyboard
  ps2kb_init();
  
  // Print welcome banner
  kprintf("Welcome on LearnixOS\n");
  
  // Enable interrupts
  arch_interrupts_enable();

  // We're done, hang this core
  arch_hcf();
}
