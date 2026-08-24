# LearnixOS: a modular UNIX-like kernel for students

LearnixOS is a didactic x86-64 kernel written in C and NASM assembly
It was built as a bachelor's thesis to make operating systems *concrete*: every subsystem is small, readable, and -- most importantly -- **swappable**, so you can compare competing designs on a real machine.

Please read the full documentation [here](docs/README.md).

## Why is Learnix different?

Educational OSes do exist, most notably xv6 and Minix3 (which I used a lot
myself). However, with Learnix, I wanted two important things:

1. **A modular design.** Core kernel components like the physical memory
   manager, the kernel heap, the schedulers and (in the future) security
   mitigations are swappable at build time.
2. **A modern build system** (meson + ninja) instead of a plain Makefile.

Because components are selected at compile time, you can see **why one algorithm
beats another** (a bitmap PMM vs the Buddy allocator, round-robin vs lottery
scheduling) **not only on the textbook but on a real system**.

## LLM usage

I'm a big believer in LLMs for making sense of complex topics, and OS dev is
certainly one. That's why the repo includes some Opencode skills to help you
follow the code. However, the project itself was developed **without agentic
coding**: by hand. 

Why? because offloading a project like this to an agent would have cost me all the learnig value and the deep understanding of OS concepts that comes with actually writing the code.
I still confronted with AI for my design and implementation decisions afterwards but **never asking to develop instead of me**.

## Build

Dependencies:

- `clang`
- `ld.lld`
- `meson`
- `ninja`
- `xorriso`
- `qemu-system-x86_64`

First fetch the dependencies (Limine, OVMF, freestanding headers):
```bash
./fetch-deps.sh
```

Then configure the build. Pick the kernel components you want to compare --
`meson.options` lists every available knob:
```bash
meson setup build -Dsched=round_robin -Dpmm=bitmap
```

Or use `setup.sh` for the default configuration.

Changed your mind? Reconfigure:

```bash
meson configure build -Dsched=lottery
```

Finally, compile and boot in QEMU:

```bash
ninja -C build run
```

For debugging:

```bash
ninja -C build gdb

(gdb) target remote localhost:1234
```

## Credits

- https://codeberg.org/Limine/limine-c-template-x86-64
- https://github.com/mpaland/printf
