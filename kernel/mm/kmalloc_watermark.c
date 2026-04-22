#include <learnix/lib/kpanic.h>
#include <learnix/lib/string.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/types.h>

static void *watermark, *end;

static inline void
kmalloc_grow ()
{
  // FIXME: kernel page table
  vmm_map ((void *)vmm_get_pgtable (), (vaddr_t)end, pmm_alloc (0), 1);
  end += 4096;
}

void
kmalloc_init (void *heap_start, size_t heap_size)
{
  watermark = heap_start;
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
