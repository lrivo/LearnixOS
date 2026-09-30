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

## VMM: Virtual Memory Manager
This component manages **page table**, the most important memory abstraction of modern OSes.

It is implemented as **architecture agnostic**, you will see it relies on `arch_map_pg` and `arch_unmap_pg()`, that each HW architecture must define to map/unmap a single virtual address.

## Kmalloc()
This is the **kernel heap**, used for dynamicly allocated structures like processes's PCBs.

Two interchangeable backends implement it (pick with `-Dkmalloc=`), each in its own folder:
- **watermark** ([`kmalloc_watermark/`](kmalloc_watermark/)): a bump allocator, no free.
- **free_list** ([`kmalloc_freelist/`](kmalloc_freelist/)): a linked free list of chunks with split and free.

> NOTE: it provides **virtual continuity**.. not physicals