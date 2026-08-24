# Features
- [x] extend the build system to support swappable components (scheduler, pmm, kmalloc)
- [x] ChaCha20 CSPRNG
- [x] file descriptors
  - [x] TTY's stdin and stdout 
  - [] actual on-disk files
  - [x] pipe() syscall
  - [x] dup() and dup2() syscalls
  - [x] lseek()
- [x] mmap() syscall
  - [x] munmap() syscall
- [] VMM needs some sort of vm_area instead of just page tables to represent things like lazy mappings and file-backed mappings
    - [] CoW fork
- [] VirtIO block driver

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
    - RDSEED support should be checked via CPUID (for now I disabled RDSEED completely to test on an old laptop)

# Security
- [x] stack canaries in userspace
    - the kernel hardcodes them, correct behaviour dictates that the libc does it
- ASLR in userspace
    - a mess, since it requires loading position indipendent code
- KASLR (enable in Limine and compile kernel as PIC)
- KPTI (kernel page table isolation) as a tunable compile option

# Bugs
- [x] userspace/brainfuck immediately kernel panics
    - FIXED by discarding .gnu.note.* sections at linker level
- [ ] weird CTRL+C problems
    - reproduce: echo "type" echo CTR+C echo <keyboard freeze>
