# Features
- extend the build system to support swappable components (scheduler, pmm, kmalloc)
- random number generator
- add file descriptors
- pipe() syscall
- mmap() syscall
- linked-list based PMM

# Security
- KASLR (enable in Limine and compile kernel as PIC)
- ASLR in userspace
- stack canaries in userspace

# Bugs
- userspace/brainfuck immediately kernel panics
