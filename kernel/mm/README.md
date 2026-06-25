# mm/ - Memory Management Subsystem

Memory management is a very important (and interesting) topic, that lives at the heart of every operating system.

It can be splitted in three layers, each relies on the lower one:
- **Physical Memory**
- **Virtual Memory**
- **kmalloc()**

## PMM: Physical Memory Manage
This reads Limine's memory map and then creates a data structure to track each physical frame (4KB).

Learnix demands that this component is **refcount-aware**, meaning that it tracks at this level if a physical page is shared between multiple memory mappings.

## VMM: Virtual Memory Manager
This component manages **page table**, the most important memory abstraction of modern OSes.

It is implemented as **architecture agnostic**, you will see it relies on `arch_map_pg` and `arch_unmap_pg()`, that each HW architecture must define to map/unmap a single virtual address.

## Kmalloc()
This is the **kernel heap**, used for dynamicly allocated structures like processes's PCBs.

> NOTE: it provides **virtual continuity**.. not physicals