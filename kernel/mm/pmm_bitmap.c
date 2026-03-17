#include "limine.h"
#include <core.h>
#include <lib/string.h>
#include <mm/memlayout.h>
#include <mm/pmm.h>
#include <stddef.h>
#include <stdint.h>

/* Total amount of physical memory in bytes on this system. */
static size_t tot_mem_byte = 0;
static size_t tot_phys_pgs = 0;
static uintptr_t bitmap_end = 0;
static uintptr_t refcount_end = 0;

/* Allocator's internal data structures. */
static uint8_t *bitmap = NULL;
static uint16_t *refcount = NULL;

/* Internal conversion helpers. */
static inline size_t pa_to_idx(physaddr_t pa) {
  return PGROUNDDOWN(pa) / PGSIZE;
}

static inline physaddr_t idx_to_pa(size_t idx) {
  return (physaddr_t)idx * PGSIZE;
}

static inline void bitmap_set_bit(size_t idx) {
  bitmap[idx / 8] |= (1 << (idx % 8));
}

static inline void bitmap_clear_bit(size_t idx) {
  bitmap[idx / 8] &= ~(1 << (idx % 8));
}

/* Returns 1 if the requested bit index is 1, otherwise 0. */
static inline int bitmap_test_bit(size_t idx) {
  return (bitmap[idx / 8] & (1 << (idx % 8))) != 0;
}

/* Returns the highest real address of physical memory 4KB page aligned */
static uint64_t memmap_find_highest_addr(struct limine_memmap_response *mm) {
  uint64_t highest = 0;
  for (uint64_t i = 0; i < mm->entry_count; ++i) {
    struct limine_memmap_entry *entry = mm->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE ||
        entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE ||
        entry->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE ||
        entry->type == LIMINE_MEMMAP_ACPI_NVS ||
        entry->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES) {
      // compute the top address of this memory region
      uint64_t top = entry->base + entry->length;
      if (top > highest)
        highest = PGROUNDUP(top);
    }
  }
  return highest;
}

static inline void assert_valid_pa(physaddr_t pa) {
  if (pa_to_idx(pa) > tot_phys_pgs) {
    kpanic("pmm: invalid pa");
  }
}

/* Sets or clears the given range of indexes. */
static void bitmap_assign_range(size_t start, size_t end, int set) {
  while (start < end) {
    set ? bitmap_set_bit(start) : bitmap_clear_bit(start);
    start++;
  }
}

/* test */
static void pmm_test() {
  // #1: should get two contiguous physical pages
  physaddr_t p1 = pmm_alloc(PMM_NONE);
  physaddr_t p2 = pmm_alloc(PMM_NONE);
  if (p1 + PGSIZE != p2) {
    kpanic("pmm_test #1: not contiguous");
  }

  // 2. free the second page, should re-allocate it
  pmm_unref_pg(p2);
  if (pmm_alloc(PMM_NONE) != p2) {
    kpanic("pmm_test #2: page not freeed correctly");
  }
}

void pmm_init(struct limine_memmap_response *mmap) {
  // we must have the memory map, panic otherwise.
  if (!mmap) {
    kpanic("pmm_init: no memory map");
  }

  /* 0) initialize the physical memory counters. */
  tot_mem_byte = memmap_find_highest_addr(mmap);
  tot_phys_pgs = tot_mem_byte / PGSIZE;

  /* 1) loop trough mmap to find a large enough region for the bitmap. */
  size_t bitmap_size = tot_phys_pgs / 8;
  size_t refcount_size = tot_phys_pgs * sizeof(uint16_t);
  for (size_t i = 0; i < mmap->entry_count; i++) {
    struct limine_memmap_entry *entry = mmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE && bitmap_size + refcount_size <= entry->length) {
      /* We allocate bitmap and refcount contiguously in physical memory. */
      bitmap = (uint8_t *)PA_TO_HHDM(entry->base);
      bitmap_end = (uintptr_t)bitmap + bitmap_size;
      refcount = (uint16_t*)bitmap_end;
      refcount_end = (uintptr_t)refcount + refcount_size;
      break;
    }
  }

  if (!bitmap || !refcount) {
    kpanic("pmm_init: failed to allocate the bitmap or refcount[]");
  }

  /* 2) pre-initialize everything as allocated (1) and zero refcount. */
  memset((void *)bitmap, 0xFF, bitmap_size);
  memset((void*)refcount, 0, refcount_size);

  /* 3) loop through mmap again to explicitly mark usable regions (0). */
  for (size_t i = 0; i < mmap->entry_count; i++) {
    struct limine_memmap_entry *entry = mmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE ||
        entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
      // OPTIMIZE: could memset in 64 bit blocks
      uint64_t start = pa_to_idx(PGROUNDDOWN(entry->base));
      uint64_t end = pa_to_idx(PGROUNDUP(entry->base + entry->length));
      bitmap_assign_range(start, end, 0);
    }
  }

  /* 4) mark the bitmap and refcount[] as allocated in the bitmap itself. */
  size_t start = pa_to_idx(PGROUNDDOWN(HHDM_TO_PA(bitmap)));
  size_t end = pa_to_idx(PGROUNDUP(HHDM_TO_PA(refcount + refcount_size)));
  bitmap_assign_range(start, end, 1);

  pmm_test();
}

physaddr_t pmm_alloc(size_t flags) {
  uint64_t *b = (uint64_t *)bitmap;
  size_t idx = 0;

  /* Scan the bitmap in 64-bit chunks, skipping fully allocated ones entirely. */
  while ((uintptr_t)b < bitmap_end && *b == PMM_ALLOC_FAIL) {
    b++;
    idx += 64;
  }

  /* If we exit the bitmap we have finished physical memory. */
  if ((uintptr_t)b >= bitmap_end) {
    return PMM_ALLOC_FAIL;
  }

  /* Othersise we've found a block with at least one free physical page, find the first one. */
  for (size_t i = 0; i < 64; i++, idx++) {
    // OPTIMIZE: instead of looping through individual bits I could use compiler
    // optimized bit finders
    if (bitmap_test_bit(idx) == 0) {
      // mark it as allocated before returning it
      bitmap_set_bit(idx);

      // initialize his refcount to 1
      refcount[idx] = 1;

      // get his physical address
      physaddr_t pa = idx_to_pa(idx);

      // apply flags
      if (flags & PMM_ZERO) {
        memset((void *)PA_TO_HHDM(pa), 0, PGSIZE);
      }

      return pa;
    }
  }

  // unreachable code, suppress warning.
  return PMM_ALLOC_FAIL;
}

// FIXME: no overflow checks
void pmm_ref_pg(physaddr_t pa) {
  assert_valid_pa(pa);

  refcount[pa_to_idx(pa)]++;
}

void pmm_unref_pg(physaddr_t pa) {
  assert_valid_pa(pa);

  size_t idx = pa_to_idx(pa);
  
  /* Can't free bitmap or refcount[] physical pages. */
  if (idx >= pa_to_idx(HHDM_TO_PA(bitmap)) && idx <= pa_to_idx(HHDM_TO_PA(refcount_end))) {
    kpanic("pmm_unref_pg: pinned pa");
  }
  
  /* Free the physical page if the refcount reaches zero. */
  if (--refcount[idx] == 0) {
    bitmap_clear_bit(idx);
  }
}
