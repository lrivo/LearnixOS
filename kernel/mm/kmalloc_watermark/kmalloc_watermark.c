#include <learnix/mm/kmalloc.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/lib/string.h>

static void *watermark, *end;

static inline void
kmalloc_grow ()
{
  // end is always in the next unmapped page
  vmm_map (vmm_get_kern_pgtable(), (vaddr_t)end, pmm_alloc (PMM_ZERO), VMM_FLAG_WRITE);
  end += 4096;
}

void
kmalloc_init (vaddr_t heap_start, size_t heap_size)
{
  vmm_map (vmm_get_kern_pgtable(), heap_start, pmm_alloc (PMM_ZERO), VMM_FLAG_WRITE);
  watermark = (void*)heap_start;
  end = (void *)((size_t)heap_start + heap_size);
}

void *
kmalloc (size_t size)
{
  void *ptr = watermark;
  watermark += size;
  if (watermark >= end)
    kmalloc_grow ();
  return ptr;
}

void *
kzalloc (size_t size)
{
  void *ptr = kmalloc (size);
  memset (ptr, 0, size);
  return ptr;
}

void
kfree (void *ptr)
{
  // do nothing
  return;
}
