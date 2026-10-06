# mm/kmalloc_watermark: Watermark kmalloc

Selected with `-Dkmalloc=watermark`.

## Overview
The simplest possible kernel heap: a **bump allocator**. There is a single `watermark` pointer that only ever moves forward — every `kmalloc()` hands out the bytes at the watermark and advances it, mapping a fresh zeroed page from the PMM (through the VMM) whenever the heap runs out.

## Implementation
- `kmalloc_init()` maps the first heap page and sets `watermark` to the heap start and `end` to the end of the initial heap region.
- `kmalloc()` returns the current watermark and bumps it by `size`; when it reaches `end`, `kmalloc_grow()` maps one more 4KB page.
- `kzalloc()` is `kmalloc()` + `memset(0)`.

## Limitations (on purpose, for teaching)
- **There is no `kfree()`**: memory is never reclaimed, so this backend is only usable for allocations that live for the whole boot.
- No alignment guarantee beyond what consecutive bumps happen to give.
- It exists so the boot path has a working heap before the freelist allocator is selected; compare with [`kmalloc_freelist/`](../kmalloc_freelist/).

## Files
- `kmalloc_watermark.c` — the whole backend
- Public prototypes live in [`learnix/mm/kmalloc.h`](../../include/learnix/mm/kmalloc.h)