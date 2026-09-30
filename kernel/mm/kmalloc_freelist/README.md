# mm/kmalloc_freelist: Free-list kmalloc

Selected with `-Dkmalloc=free_list`.

## Overview
This is the "real" kernel heap: `kmalloc()` and `kfree()` over a **singly linked free list** of chunks. The heap starts as a handful of pages mapped by `kmalloc_init()` and grows on demand by mapping more pages from the PMM through the VMM.

## Implementation
Memory is carved into `struct chunk` headers laid out contiguously (a chunk's successor is `header + 8 + size`, see `CNK_NXT()`), each carrying:
- `flags` — currently only `CNK_ALLOC` (in use or free)
- `size` — payload size
- `next` — link used only while the chunk is on the free list

The `struct heap` keeps a `wilderness` pointer (end of the mapped heap) and the `freelist` head.

- **Alloc** walks the free list and splits the first chunk big enough; allocation just sets `CNK_ALLOC`.
- **Free** clears the flag and pushes the chunk back on the freelist (no coalescing yet — see `todo.md`).
- List removal uses the pointer-to-pointer idiom (`list_remove()`), the "Linus Torvalds' good taste" version that needs no special-casing of the head.

## Files
- `kmalloc_freelist.c` — allocator implementation
- `kmalloc_freelist.h` — `struct chunk` / `struct heap` and chunk macros (private to this backend)
- The public `kmalloc()` / `kzalloc()` / `kfree()` prototypes live in [`learnix/mm/kmalloc.h`](../../include/learnix/mm/kmalloc.h)

## References
- [Doubly linked list "good taste" idiom](https://grisha.org/blog/2013/04/02/linus-torvalds-admires-good-code/)