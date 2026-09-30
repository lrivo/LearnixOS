#include "kmalloc_freelist.h"
#include <learnix/mm/kmalloc.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>
#include <learnix/lib/string.h>

static struct heap heap_state = { 0 };

// TODO: make this a kernel library, as we use linked list often
static inline void
list_remove (struct chunk **head, struct chunk *target)
{
  // Linus Torvald's good taste solution from a TED Talk he did
  struct chunk **p = head;
  while (*p != target)
    p = &(*p)->next;
  *p = target->next;
}

void
kmalloc_init(vaddr_t heap_start, size_t heap_size)
{
  (void)heap_size;

  for (int i = 0; i < 10; i++) {
      vmm_map(
        vmm_get_kern_pgtable(), heap_start + (i * 4096), pmm_alloc(PMM_ZERO), VMM_FLAG_WRITE
      );
  }

  // memory map a 4KB page initially
  
  // initialize the freelist
  struct chunk *freelist = (struct chunk*)heap_start;
  freelist->flags = 0;
  freelist->size =  (10 * 4096) - sizeof(struct chunk) + 8;
  freelist->next = NULL;

  // initial heap_state
  heap_state.wilderness = heap_state.freelist = freelist;
  heap_state.nchunks = 1;

  kprintf("[INFO] kmalloc_freelist initialized\n");
}

void*
kmalloc(size_t size)
{
  // round up size
  if (size < ALLOC_MIN) size = ALLOC_MIN;
  
  // walk the freelist, searching for a large enough chunk
  struct chunk *curr = heap_state.freelist;
  while (curr)
  {
    if (curr->size >= size) break;
    curr = curr->next;
  }
 
  // TODO: try to grow the kheap
  if (!curr) 
    kpanic("kmalloc: out of memory"); 
  
  // remove curr from the freelist
  list_remove(&heap_state.freelist, curr);

  /* we can split if curr's effective size can contain
   * another chunk with at least 8 bytes of data */ 
  if (curr->size > size && curr->size - size >= sizeof(struct chunk))
  {
    // cache curr's old size
    uint32_t old_sz = curr->size;
    curr->size = size;

    // initialize the remainder chunk
    struct chunk *remainder = CNK_NXT(curr);
    remainder->flags = 0;
    remainder->size = old_sz - size - 8;

    // remainder becomes the head of freelist
    remainder->next = heap_state.freelist;
    heap_state.freelist = remainder;

    // if we splitted the wilderness chunk, update it
    if (curr == heap_state.wilderness)
      heap_state.wilderness = remainder;

    heap_state.nchunks++;
  }

  curr->flags = CNK_ALLOC;
  curr->next = NULL;
  heap_state.nalloc++;

  return (char*)curr + 8;
}

void*
kzalloc(size_t size)
{
  void *ptr = kmalloc(size);
  memset(ptr, 0, size);
  return ptr;
}

void
kfree(void *ptr)
{
  struct chunk *cnk = CNK_HDR(ptr);
  if (!ptr || !IS_CNK_ALLOC(cnk))
    kpanic("kfree: double free");

  struct chunk *nxt = CNK_NXT(cnk);
  while (!IS_CNK_ALLOC(nxt))
  {
    list_remove(&heap_state.freelist, nxt);
    cnk->size += nxt->size + 8;
    heap_state.nchunks--;

    if (nxt == heap_state.wilderness)
    {
      heap_state.wilderness = cnk;
      break;
    }

    nxt = CNK_NXT(nxt);
  }

  cnk->flags = 0;
  cnk->next = heap_state.freelist;
  heap_state.freelist = cnk;
  heap_state.nalloc--;
}
