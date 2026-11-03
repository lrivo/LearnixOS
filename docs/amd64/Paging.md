# x86-64 Paging

Paging translates virtual addresses to physical frames and applies access permissions. Learnix keeps the page-table operations behind the generic VMM interface; the x86-64 implementation is in [`paging.c`](../../kernel/arch/amd64/paging.c) and [`paging.h`](../../kernel/arch/amd64/paging.h).

## Address translation

The current walker uses four page-table levels. Each level has 512 eight-byte entries; the four index fields are nine bits each, followed by the twelve-bit offset within a 4 KiB page.

| Field | Virtual-address bits | Index macro |
|---|---:|---|
| PML4 | 47–39 | `PML4_IDX(va)` |
| PML3 | 38–30 | `PML3_IDX(va)` |
| PML2 | 29–21 | `PML2_IDX(va)` |
| PML1 / PTE | 20–12 | `PML1_IDX(va)` |
| Page offset | 11–0 | — |

`pgdirwalk()` follows the PML4 → PML3 → PML2 → PML1 chain and returns the leaf entry for a 4 KiB mapping. When mapping, it creates missing intermediate tables using zeroed physical frames from the PMM and marks each intermediate entry present and writable. Page-table physical addresses are converted to HHDM virtual addresses with `P2V()` so the kernel can access the tables.

The walker also recognizes a huge-page bit at PML3 or PML2 and stops at that entry. The public mapping, unmapping, and translation paths are implemented for 4 KiB leaves; they do not provide complete huge-page management or offset handling.

## Page-table roots and CR3

`arch_get_pgtable()` reads the physical root address from `CR3`, masks off its low 12 bits to isolate the page-aligned root, and converts the root to an HHDM pointer. `vmm_init()` saves the active root as the kernel page table. New process address spaces allocate a root page and copy the kernel's top-level PML4 entries; a context switch converts the selected root back to a physical address and writes it to `CR3`.

This is where architecture-specific paging meets the generic VMM: [`vmm.c`](../../kernel/mm/vmm.c) rounds mapping addresses to page boundaries and calls `arch_pg_map()`, `arch_pg_unmap()`, and `arch_va_to_pa()`. See [`uvm.c`](../../kernel/mm/uvm.c) for process-root creation and [`process.c`](../../kernel/arch/amd64/process.c) for CR3 updates during process changes.

## Mapping and permissions

`arch_pg_map()` stores the page-aligned physical address and builds the leaf PTE flags. Every mapping is present and readable; by default it is not writable and has NX set. After writing the leaf PTE, it invalidates that virtual address in the TLB with `invlpg`. The architecture-independent flags are translated as follows:

| VMM flag | AMD64 PTE effect |
|---|---|
| `VMM_FLAG_WRITE` | Sets `PTE_WRITE` on the leaf |
| `VMM_FLAG_EXEC` | Clears `PTE_NX` on the leaf |
| `VMM_FLAG_USER` | Sets `PTE_USER` in all four levels, including the leaf |
| `VMM_FLAG_NOCACHE` | Sets `PTE_PWT` and `PTE_PCD` on the leaf |
| `VMM_FLAG_GLOBAL` | Declared by the generic interface, but not translated by this mapping implementation |

The kernel enables `EFER.NXE` in `arch_stage_1()`, which is the prerequisite for using `PTE_NX` in leaf entries. That stage also enables SMEP and SMAP when CPUID reports support. See [`cpu.c`](../../kernel/arch/amd64/cpu.c) for the setup.

## Unmapping and translation lookup

`arch_pg_unmap()` walks without allocating missing levels, clears the leaf's present bit, invalidates the virtual address in the TLB with `invlpg`, and unreferences the mapped physical frame. It also unreferences intermediate page-table frames when their entries are all non-present.

`arch_va_to_pa()` performs a non-allocating walk. It returns zero if a level or the leaf is not present; otherwise it combines the mapped physical page base with the original 12-bit page offset.

The generic VMM assumes mappings are present and readable. Its `vmm_map()` wrapper can allocate a physical frame when passed a null physical address, while the architecture-specific mapper handles the page-table entries and permissions. See the [memory-management guide](../../kernel/mm/README.md) for how page tables fit into the wider VMM and process address-space model.
