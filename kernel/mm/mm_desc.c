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

  mm->pgtable = uvm_alloc ();

  return mm;
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
