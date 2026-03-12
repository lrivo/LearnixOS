#include "lib/kprintf.h"
#include <drivers/console.h>
#include <interrupts.h>
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

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used,
               section(".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

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
  
  kprintf("Hello World!\n%d!", 10);

  // Initialize the CPU
  arch_stage_1();

  // We're done, hang this core
  arch_hcf();
}
