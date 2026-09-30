# mm/pmm_bitmap: Bitmap PMM

Selected with `-Dpmm=bitmap` (the default).

## Overview
This is the bitmap backend of the Physical Memory Manager. It reads Limine's memory map and tracks every usable 4KB physical frame with **one bit each**: `1` means the frame is allocated, `0` means it is free.

It is **refcount-aware**: alongside the bitmap it keeps a `uint16_t` per frame so that shared mappings (e.g. after `fork()`) can be accounted at the PMM level. The bitmap and the refcount array are carved out of the first usable frames found in the memory map.

## Implementation
Allocation is a **first-fit scan**: walk the bitmap bit by bit until a run of free frames matching the request is found. Freeing just clears the corresponding bits and decrements the refcounts.

- The bitmap size is `total_frames / 8` bytes — 1/8 of one frame per frame of RAM, a fixed and tiny overhead.
- Frame index <-> physical address conversion is a simple `PGSIZE` multiply/divide (`pa_to_idx()` / `idx_to_pa()`).
- Single-frame alloc/free are O(n) in the worst case (scan from the start), which is the trade-off accepted in exchange for **contiguous multi-frame allocations**, something the freelist backend can't do efficiently.

## Files
- `pmm_bitmap.c` — the whole backend; implements the interface declared in [`learnix/mm/pmm.h`](../../include/learnix/mm/pmm.h)

## References
- [OSTEP — Memory API / free-space management](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-papers.html)