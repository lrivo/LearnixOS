# Exceptions and interrupts

On AMD64, the CPU uses the **Global Descriptor Table (GDT)** to interpret segment selectors and the **Interrupt Descriptor Table (IDT)** to find the entry point for an exception or interrupt vector. Learnix sets up both during early CPU initialization, before enabling maskable interrupts.

Implementation: [`gdt.c`](../../kernel/arch/amd64/gdt.c), [`idt.c`](../../kernel/arch/amd64/idt.c), and [`isr_stubs.asm`](../../kernel/arch/amd64/isr_stubs.asm).

## GDT: segments and the ring-transition stack

In 64-bit mode, segment bases and limits do not provide the usual memory segmentation; the descriptors still matter for privilege level, code/data attributes, and selector values. Learnix builds this GDT in [`gdt.h`](../../kernel/arch/amd64/gdt.h):

| Selector | Descriptor | Use |
|---|---|---|
| `0x00` | Null | Required null entry |
| `0x08` | Kernel code | Ring 0 code; also the selector installed in IDT gates |
| `0x10` | Kernel data | Ring 0 data/stack |
| `0x18` | User data | Ring 3 data/stack |
| `0x20` | User code | Ring 3 code |
| `0x28` | TSS | Task State Segment descriptor |

`gdt_init()` fills in these descriptors, constructs a 64-bit TSS descriptor, and prepares the TSS. It loads the table with `lgdt`, then calls the assembly `reloadSegments` routine to reload `CS` via a far return and reload the data segment registers. It loads the TSS selector with `ltr` afterward.

The TSS is used here for its `RSP0` stack pointer, not hardware task switching. When an interrupt changes privilege from ring 3 to ring 0, the CPU takes the kernel stack from `TSS.RSP0` before saving the user execution state. Learnix updates that field when it selects a process, so the next user-to-kernel transition lands on that process's kernel stack. `gdt_init()` sets the TSS I/O-map offset to the size of the TSS entry.

## IDT: vector-to-stub table

The IDT has 256 possible entries. Each 16-byte gate stores a handler address split across three fields, a code-segment selector, an IST field, and type/attribute bits. `idt_load()` forms the IDTR limit and base and loads it with `lidt`.

Learnix currently creates assembly stubs for vectors `0` through `33`:

- `0`–`31`: processor exception vectors (including reserved entries)
- `32` (`0x20`): LAPIC timer interrupt
- `33` (`0x21`): PS/2 keyboard interrupt

The remaining IDT entries are not populated by the current stub table. The IDT gate points to a small assembly stub, not directly to the C handler. The stub normalizes the stack by pushing the vector number and either the CPU-provided error code or a dummy zero, saves general-purpose registers, and calls `intr_dispatcher()` with the resulting [`intr_trap_frame`](../../kernel/arch/amd64/include/learnix/arch/interrupts.h).

For a ring-3 event, the CPU switches to `RSP0` and saves the prior `SS:RSP` along with `RFLAGS`, `CS`, and `RIP` (and an error code for exceptions that provide one). For an event while already in ring 0, the CPU does not push a new `SS:RSP`; those fields in the C frame are therefore not a saved kernel stack pair for same-privilege events. The assembly checks the saved `CS` and executes `swapgs` only for user-to-kernel entry. After the handler returns, the common `kernel_exit` path restores registers and returns with `iretq`.

The exception stubs must agree with the CPU about whether an error code is present, or the rest of the saved frame is misread. The assembly uses the error-code stub for vectors `8`, `10`–`14`, `17`, and `21`; the other vectors use a dummy error value. On AMD64, #VC (29) and #SX (30) also provide hardware error codes, but their current entries use the no-error stub. If either exception occurs, the frame is misaligned and the return path cannot safely use it. The gate table is 256 entries, but the assembly stub table contains only 34 entries, so this is not a general-purpose registration path for arbitrary vectors.

## Handler registration and dispatch

`idt_init()` registers stubs for `0`–`33` with no C handler, installs the page-fault handler at vector `14`, installs the timer handler at `0x20`, and finally loads the IDT. The PS/2 driver later registers its handler at `0x21`. When a vector fires, `intr_dispatcher()` indexes the C handler array by the saved vector and calls that handler. If none is registered, the kernel panics.

The page-fault handler reads the faulting virtual address from `CR2`. It translates the x86 page-fault error bits (present/protection, write, user, and instruction fetch) to the architecture-independent `mm_page_fault()` flags. A fault the MM layer classifies as a kernel fault panics; a process fault exits the current process; a handled fault returns to the interrupted context.

The timer handler runs `sched_tick()` and sends an end-of-interrupt (EOI) to the local APIC. Device interrupts use APIC routing in addition to the IDT: a device/controller must route an interrupt to the vector whose IDT entry has the intended stub.

The AMD64 implementation currently builds every registered gate as a ring-0 interrupt gate (`0x8E`, selector `0x08`). Its `arch_interrupts_register()` implementation does not use the generic `flags` argument, so the generic user-callable and trap-gate options do not currently change the hardware gate attributes.

## Configuration flow during boot

`kmain()` begins with maskable interrupts disabled. After virtual memory setup it calls `arch_stage_1()`, which establishes exception handling early:

1. `gdt_init()` builds and loads the GDT, reloads segment registers, and loads the TSS.
2. `idt_init()` installs the exception/interrupt stubs and the initial page-fault and timer handlers, then executes `lidt`.
3. The rest of `arch_stage_1()` enables the NX bit and enables SMEP/SMAP when CPUID reports support.
4. Later, `arch_stage_2()` initializes the HPET, LAPIC, and IOAPIC. The PS/2 driver registers vector `0x21` after this stage.
5. The kernel enables maskable interrupts in the idle loop, after initialization, so the configured timer can drive scheduling.

The boot order is visible in [`main.c`](../../kernel/main.c); the two architecture stages are in [`cpu.c`](../../kernel/arch/amd64/cpu.c). The early IDT means exceptions can be caught before timer and device routing are initialized; it does not mean those device interrupts are ready to arrive yet.
