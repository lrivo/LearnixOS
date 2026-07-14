# Features
- [x] extend the build system to support swappable components (scheduler, pmm, kmalloc)
- [x] ChaCha20 CSPRNG
- [x] file descriptors
  - [x] TTY's stdin and stdout 
  - [] actual on-disk files
  - [] pipe() syscall
- [x] mmap() syscall
  - [x] munmap() syscall

# Components
- linked-list based PMM
- buddy PMM
- MLFQ schedueler

# Improvements & Refactors
- ELF loading
    - elf_load() implicitly assumes all section are 4KB
    - elf_load() does not check if a section is mapped in a page of another previous section (brainfuck bug)
- PMM
    - expose flags to request specific type of memory (eg: DMA ready, below XGB, ...)
- VMM
    - mmap() and munmap() code is very very bad
- CPU
    - arch_cpu_get() should be replaced with some shorter macro
    - struct cpu should probably contain the runqueue for SMP reasons
    - amd64 CPUID parsing is never used at all

# Security
- [x] stack canaries in userspace
- ASLR in userspace
    - a mess, since it requires loading position indipendent code
- KASLR (enable in Limine and compile kernel as PIC)
- KPTI (kernel page table isolation) as a tunable compile option

# Bugs
- [x] userspace/brainfuck immediately kernel panics
    - FIXED by discarding .gnu.note.* sections at linker level
- [ ] weird CTRL+C problems
    - reproduce: echo "type" echo CTR+C echo <keyboard freeze>
