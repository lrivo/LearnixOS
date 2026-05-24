# Features
- [x] extend the build system to support swappable components (scheduler, pmm, kmalloc)
- random number generator
- add file descriptors
- pipe() syscall
- mmap() syscall
- linked-list based PMM

# Improvements
- elf_load() implicitly assumes all section are 4KB

# Security
- [x] stack canaries in userspace
- ASLR in userspace
- KASLR (enable in Limine and compile kernel as PIC)

# Bugs
- userspace/brainfuck immediately kernel panics
    - caused by the ELF re-mapping the same page as read only due to his ELF file
