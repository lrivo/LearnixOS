# LearnixOS: a modular UNIX-like kernel for students
LearnixOS started as my bachelor's thesis project to deep dive into operating systems in a practical way, but later evolved in an effort to create the **best educational kernel** with a **modular** approach. <br>

Please read the full documentation [here](docs/README.md).

## Modularity
Modularity means that LearnixOS supports, and includes, different implementations for the same kernel component. <br>
For example, the Physical Memory Manager, might be implemented with:
- a bitmap
- a linked list of free frames
- a buddy allocator

Existing educational resources, like xv6, will only show **one possibility** while Learnix aims to show all of them. <br>
I'd argue that seeing, and most importantly **trying** more implementations, allows one to understand why modern systems use a specif one.

## LLM usage
I am a big believer in LLMs for helping making sense of complex topics, and OsDev is certainly one. <br>
However, for this project, I have one personal rule:
> don't delegate important tasks to a coding agent

For example:
- ❌ "Implement the SLAB allocator as a swappable kernel heap"
- ✅ "Create a new userspace program called xxx"

Documentation is mostly LLM-written for now, but I'll try to rewrite the important parts myself.

## Build

### Dependencies
Make sure to install those packages:
- `clang`
- `ld.lld`
- `meson`
- `ninja`
- `xorriso` (doesn't exist on MacOS)
- `qemu-system-x86_64`

First fetch the dependencies (Limine, OVMF, freestanding headers):
```bash
./fetch-deps.sh
```

### Configuration
Then configure the build with the kernel components you prefer:
```bash
meson setup <build-folder> -Dsched=xxx -Dpmm=xxx -Dkheap=xxx
```
Check `meson.option` to see all the available components.

Or just use `setup.sh` for the default configuration.

### Compile and boot in QEMU
```bash
ninja -C <build-folder> run
```

For debugging:

```bash
ninja -C <build-folder> gdb

(gdb) target remote localhost:1234
(gdb) file <build-folder>/kernel/kernel.elf
```

## Credits
- https://codeberg.org/Limine/limine-c-template-x86-64
- https://github.com/mpaland/printf
