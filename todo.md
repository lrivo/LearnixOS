# Features
- [x] extend the build system to support swappable components (scheduler, pmm, kmalloc)
- [x] ChaCha20 CSPRNG
- add file descriptors
- pipe() syscall
- mmap() syscall
- linked-list based PMM

# Improvements
- elf_load() implicitly assumes all section are 4KB
- elf_load() does not check if a section is mapped in a page of another previous section (brainfuck bug)

# Security
- [x] stack canaries in userspace
- ASLR in userspace
- KASLR (enable in Limine and compile kernel as PIC)

# Bugs
- [x] userspace/brainfuck immediately kernel panics
    - FIXED by discarding .gnu.note.* sections at linker level
- [ ] weird CTRL+C problems
    - reproduce: echo "type" echo CTR+C echo <keyboard freeze>
