#include "learnix/mm/mm_types.h"
#include <learnix/mm/mm.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/mm/vma.h>
#include <learnix/mm/vmm.h>

/* NOTE: for now mm_alloc() wraps uvm_alloc() and mm_free() wraps
 * uvm_destroy(). In a later step the uvm code will be absorbed
 * here, since struct mm_desc is the same thing but more complete. */

struct mm_desc *
mm_alloc (void)
{
  // kzalloc zeroes vma_head and all the image layout fields
  struct mm_desc *mm = kzalloc (sizeof (struct mm_desc));
  if (!mm)
    return NULL;

  mm->pgtable = uvm_alloc();

  return mm;
}

int
mm_copy(struct mm_desc *old, struct mm_desc *new) {
    if (!old || !new || old == new) return -1;

    // duplicate VMAs
    for (struct vm_area *v = old->vma_head; v != NULL; v = v->next) {
        struct vm_area *dup = vma_alloc(v->start, v->end, v->flags);
        if (v->flags & VMA_FILEBACKED) {
            dup->backing = v->backing;
            dup->backing_offset = v->backing_offset;
            dup->backing_size = v->backing_size;
        }
        vma_insert(&new->vma_head, dup);
    }

    /* duplicate present pages.
     * NOTE: don't remove this, otherwise the child will have
     * an empty stack and lose .data/.bss overwritten parts
     * by his parent. */
    uvm_copy(new->pgtable, old->pgtable);

    return 0;
}

void
mm_free (struct mm_desc *mm)
{
  if (!mm)
    return;

  // first walk the VMA list and free them one by one
  struct vm_area *curr = mm->vma_head;
  while (curr) {
    struct vm_area *next = curr->next;
    vma_free (curr);
    curr = next;
  }

  // then the page table and the descriptor itself
  uvm_destroy (mm->pgtable);
  kfree (mm);
}
