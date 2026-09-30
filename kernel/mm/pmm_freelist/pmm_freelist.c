#include "pmm_freelist.h"
#include <learnix/arch/memlayout.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>
#include <learnix/lib/string.h>
#include <learnix/mm/pmm.h>
#include <learnix/types.h>
#include <limine.h>

/* Total amount of physical memory in bytes on this system. */
static size_t tot_mem_byte = 0;
static size_t tot_phys_pgs = 0;
static uintptr_t pages_end = 0;

/* Array of struct page sorted by physical address, for fast indexing. */
static struct page *pages = NULL;
/* Head of the freelist. */
static struct page *freelist_head = NULL;

/* Internal conversion helpers. */
static inline size_t
pa_to_idx(paddr_t pa)
{
  return PGROUNDDOWN(pa) / PGSIZE;
}

static inline paddr_t
idx_to_pa(size_t idx)
{
  return (paddr_t)idx * PGSIZE;
}

/* Physical address of a struct page, no need for the caller to do pointer
 * arithmetic on the pages[] array. */
static inline paddr_t
pg_to_pa(struct page *pg)
{
  return idx_to_pa(pg - pages);
}

/* Returns the highest real address of physical memory 4KB page aligned */
static uint64_t
memmap_find_highest_addr(struct limine_memmap_response *mm)
{
  uint64_t highest = 0;
  for (uint64_t i = 0; i < mm->entry_count; ++i)
  {
    struct limine_memmap_entry *entry = mm->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE
        || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE
        || entry->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE
        || entry->type == LIMINE_MEMMAP_ACPI_NVS
        || entry->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES)
    {
      // compute the top address of this memory region
      uint64_t top = entry->base + entry->length;
      if (top > highest)
        highest = PGROUNDUP(top);
    }
  }
  return highest;
}

static inline void
assert_valid_pa(paddr_t pa)
{
  if (pa_to_idx(pa) > tot_phys_pgs)
    kpanic("pmm: invalid pa");
}

/* Pushes a frame at the head of the free list, O(1). */
static inline void
page_push(struct page *pg)
{
  pg->prev = NULL;
  pg->next = freelist_head;
  if (freelist_head)
    freelist_head->prev = pg;
  freelist_head = pg;
}

/* Pops the head of the free list, O(1). Returns NULL if memory is over. */
static inline struct page *
page_pop(void)
{
  struct page *pg = freelist_head;
  if (!pg)
    return NULL;

  freelist_head = pg->next;
  if (freelist_head)
    freelist_head->prev = NULL;

  pg->next = pg->prev = NULL;
  return pg;
}

/* O(1) unlinking, this is why the free list is doubly linked: the contiguous
 * allocator has to carve pages out of the middle of the list. */
static inline void
page_unlink(struct page *pg)
{
  if (pg->prev)
    pg->prev->next = pg->next;
  else
    freelist_head = pg->next;

  if (pg->next)
    pg->next->prev = pg->prev;

  pg->next = pg->prev = NULL;
}

/* test */
static void
pmm_test()
{
  // #1: free then alloc, with an unsorted LIFO list we must get back the same
  // page (push at head, pop at head)
  paddr_t p1 = pmm_alloc(PMM_NONE);
  pmm_unref_pg(p1);
  if (pmm_alloc(PMM_NONE) != p1)
  {
    kpanic("pmm_test #1: page not freed correctly");
  }

  // #2: refcounting, a shared page is freed only by the last unref
  pmm_ref_pg(p1);
  pmm_unref_pg(p1);
  if (pmm_alloc(PMM_NONE) == p1)
  {
    kpanic("pmm_test #2: page freed while still referenced");
  }
  pmm_unref_pg(p1);
  if (pmm_alloc(PMM_NONE) != p1)
  {
    kpanic("pmm_test #2: page not freed correctly");
  }

  // #3: contiguous allocation, free the region and get it back
  paddr_t region = pmm_alloc_cont(4, PMM_NONE);
  for (size_t i = 0; i < 4; i++)
    pmm_unref_pg(region + i * PGSIZE);
  if (pmm_alloc_cont(4, PMM_NONE) != region)
  {
    kpanic("pmm_test #3: contiguous region not freed correctly");
  }
}

void
pmm_init(struct limine_memmap_response *mmap)
{
  // we must have the memory map, panic otherwise.
  if (!mmap)
    kpanic("pmm_init: no memory map");

  /* 0) initialize the physical memory counters. */
  tot_mem_byte = memmap_find_highest_addr(mmap);
  tot_phys_pgs = tot_mem_byte / PGSIZE;

  /* 1) loop through mmap to find a large enough region for the pages[] array. */
  size_t pages_size = tot_phys_pgs * sizeof (struct page);
  for (size_t i = 0; i < mmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = mmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE && pages_size <= entry->length)
    {
      pages = (struct page *)P2V(entry->base);
      pages_end = (uintptr_t)pages + pages_size;
      break;
    }
  }

  if (!pages)
    kpanic("pmm_init: failed to allocate the pages[] array");

  /* 2) pre-initialize every frame as allocated and pinned, so that unusable
   * regions, the kernel image and the pages[] array itself can never end up
   * on the free list nor be freed. */
  for (size_t i = 0; i < tot_phys_pgs; i++) {
    pages[i].flags = PG_ALLOC | PG_PINNED;
    pages[i].refcount = 0;
    pages[i].next = pages[i].prev = NULL;
  }

  /* 3) loop through mmap again to push the free frames on the list, skipping
   * the frames occupied by the pages[] array itself. */
  size_t arr_start = pa_to_idx(PGROUNDDOWN(V2P(pages)));
  size_t arr_end = pa_to_idx(PGROUNDUP(V2P(pages_end)));
  for (size_t i = 0; i < mmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = mmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE
        || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE)
    {
      size_t start = pa_to_idx(PGROUNDDOWN(entry->base));
      size_t end = pa_to_idx(PGROUNDUP(entry->base + entry->length));
      for (size_t idx = start; idx < end; idx++)
      {
        /* Avoid clearing PG_ALLOC and PG_PINNED on the pages array.
         * No need to check for kernel code/data, as Limine already marks them
         * as unusable regions, so this loops skips them. */
        if (idx >= arr_start && idx < arr_end)
          continue;

        // mark page usable and push it on the freelist
        pages[idx].flags = 0;
        page_push(&pages[idx]);
      }
    }
  }

  pmm_test();

  kprintf("[INFO] pmm_freelist initialized\n");
}

paddr_t
pmm_alloc(size_t flags)
{
  // pop the freelist, if NULL we are out of memory
  struct page *pg = page_pop();
  if (!pg)
    return PMM_ALLOC_FAIL;

  // mark it as allocated and initialize his refcount to 1
  pg->flags |= PG_ALLOC;
  pg->refcount = 1;

  // get his physical address
  paddr_t pa = pg_to_pa(pg);

  // apply flags
  if (flags & PMM_ZERO)
    memset((void *)P2V(pa), 0, PGSIZE);

  return pa;
}

paddr_t
pmm_alloc_cont(size_t n, size_t flags)
{
  /* The free list is unsorted, so there is no way to know if n frames are
   * adjacent by walking it: we scan the pages[] array looking for a run of n
   * consecutive free frames, then carve it out of the list one page at a
   * time. Each unlink is O(1) thanks to the doubly linked list. */
  size_t run = 0;
  for (size_t idx = 0; idx < tot_phys_pgs; idx++)
  {
    if (pages[idx].flags & PG_ALLOC)
    {
      run = 0;
      continue;
    }

    if (++run < n)
      continue;

    size_t start = idx - n + 1;
    for (size_t i = start; i <= idx; i++)
    {
      page_unlink(&pages[i]);
      pages[i].flags |= PG_ALLOC;
      pages[i].refcount = 1;
    }

    paddr_t pa = idx_to_pa(start);

    // apply flags
    if (flags & PMM_ZERO)
    {
      memset((void *)P2V(pa), 0, n * PGSIZE);
    }

    return pa;
  }

  return PMM_ALLOC_FAIL;
}

// FIXME: no overflow checks
void
pmm_ref_pg(paddr_t pa)
{
  assert_valid_pa(pa);
  pages[pa_to_idx(pa)].refcount++;
}

void
pmm_unref_pg(paddr_t pa)
{
  assert_valid_pa(pa);
  size_t idx = pa_to_idx(pa);

  // panic when trying to free a pinned page frame
  if (pages[idx].flags & PG_PINNED)
    kpanic("pmm_unref_pg: pinned pa");

  // free the page frame when the refcound reaches zero
  if (--pages[idx].refcount == 0)
  {
    pages[idx].flags &= ~PG_ALLOC;  // clear PG_ALLOC
    page_push(&pages[idx]);        // freelist head push
  }
}
