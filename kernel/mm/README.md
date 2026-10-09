# mm/ - Memory Management Subsystem

Memory management is a very important (and interesting) topic, that lives at the heart of every operating system.

It can be splitted in three layers, each relies on the lower one:
- **Physical Memory**
- **Virtual Memory**
- **kmalloc()**

## PMM: Physical Memory Manage
This reads Limine's memory map and then creates a data structure to track each physical frame (4KB).

Learnix demands that this component is **refcount-aware**, meaning that it tracks at this level if a physical page is shared between multiple memory mappings.

Two interchangeable backends implement this interface (pick with `-Dpmm=`), each in its own folder:
- **bitmap** ([`pmm_bitmap/`](pmm_bitmap/)): one bit per frame, first-fit scan at allocation time.
- **freelist** ([`pmm_freelist/`](pmm_freelist/)): a per-frame `struct page { flags, refcount, next, prev }` array and an **unsorted (LIFO) doubly linked list** of free frames. Single-frame allocation *and* freeing are both O(1) (pop/push at the head); contiguous allocation is the trade-off: it must scan the whole list and unlink nodes one by one.

### Allocation statistics
Both backends feed the same, backend-independent statistics module ([`pmm_stats.c`](pmm_stats.c)). Two non-POSIX syscalls expose it to userspace:
- `SYS_PMM_STATS_START` opens a measurement window and resets the counters.
- `SYS_PMM_STATS_GET` closes it and copies a `struct pmm_stats` with `{count, min, mean, median, max}` cycles for `pmm_alloc()` and `pmm_unref_pg()` separately.

The TSC is read only while a window is open, so the allocator pays nothing when no measurement is running. This is how the bitmap/freelist A/B is measured (`userspace/bench`).

## VMM: Virtual Memory Manager
This component manages **page table**, the most important memory abstraction of modern OSes.

It is implemented as **architecture agnostic**, you will see it relies on `arch_map_pg` and `arch_unmap_pg()`, that each HW architecture must define to map/unmap a single virtual address.

## The Memory Descriptor
While the VMM maps single pages, a *process* needs a higher-level object that owns its whole address space: [`struct mm_desc`](../include/learnix/mm/mm_types.h), Learnix's version of Linux's `mm_struct`. It groups:
- `pgtable` - the root of the page table implementing this address space.
- `vma_head` - a sorted linked list of **VMAs** ([`vma.c`](vma.c)), each describing a region (`start`..`end`, permissions, file-backed or not) that makes demand paging and CoW possible.
- the layout of the loaded image (`.text`/`.data`/`.bss` bounds) plus `brk_start`/`brk_curr` for the heap.

The ownership chain is: `struct process` → `struct mm_desc` → `{pgtable, VMAs}`. The kernel stack is deliberately **not** part of the descriptor: it is a per-process resource (see [`proc/`](../proc/README.md)), while an `mm_desc` describes a shareable address space. The lifecycle lives in [`mm_desc.c`](mm_desc.c): `mm_alloc()` builds a fresh address space and `mm_free()` releases every VMA, the page table, then the descriptor itself — which also means VMAs can never leak on process exit anymore.

The VMA types and flags live in `mm_types.h` (types only), while the function prototypes are split between `vma.h` (VMA list operations) and `mm.h` (descriptor lifecycle), mirroring Linux's `<linux/mm_types.h>` / `<linux/mm.h>` split.

> NOTE: for now `mm_alloc()`/`mm_free()` still wrap the older `uvm_alloc()`/`uvm_destroy()` ([`uvm.c`](uvm.c)); the plan is to absorb the uvm code into the descriptor as the fork/exec rewrites land.

## Kmalloc()
This is the **kernel heap**, used for dynamicly allocated structures like processes's PCBs.

Two interchangeable backends implement it (pick with `-Dkmalloc=`), each in its own folder:
- **watermark** ([`kmalloc_watermark/`](kmalloc_watermark/)): a bump allocator, no free.
- **free_list** ([`kmalloc_freelist/`](kmalloc_freelist/)): a linked free list of chunks with split and free.

> NOTE: it provides **virtual continuity**.. not physicals