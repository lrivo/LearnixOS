# mm/pmm_freelist: Freelist PMM

Selected with `-Dpmm=freelist`.

## Overview
The linked-list backend of the Physical Memory Manager. Instead of one bit per frame it keeps a per-frame `struct page { flags, refcount, next, prev }` array and links all free frames into an **unsorted (LIFO) doubly linked list**.

It is **refcount-aware** like the bitmap backend: the refcount lives directly in each frame's `struct page`.

## Implementation
- **Single-frame alloc** is O(1): pop the head of the free list.
- **Single-frame free** is O(1): push the frame back at the head.
- **Contiguous multi-frame alloc** is the trade-off: since the list is unsorted we scan the `pages[]` array looking for a run of n consecutive free frames, then unlink those pages one by one. Each unlink is O(1) — that's why the list is doubly linked — so carving a region costs O(n) with a tiny constant. Learnix doesn't use contiguous allocations anyway, they exist for DMA/I/O.
- The free list is deliberately **unsorted**: keeping it sorted would add latency on every free for a property that only the (rarely used) contiguous path would benefit from.

Compare with the bitmap backend in [`pmm_bitmap/`](../pmm_bitmap/), which is the opposite trade-off.

## Files
- `pmm_freelist.c` — the backend (implements the interface declared in [`learnix/mm/pmm.h`](../../include/learnix/mm/pmm.h))
- `pmm_freelist.h` — `struct page { flags, refcount, next, prev }` and the `PG_ALLOC` flag (private to this backend)