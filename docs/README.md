# Learnix Kernel's Documentation
This documentation describes the internal architecture of the kernel. It is written to be useful both as a reference for the developer and as a learning resource for OS students approaching kernel development for the first time.

## Overview
The Learnix kernel is a UNIX-like monolithic kernel for the x86-64 architecture (at least for now) completely written in C and NASM assembly.

It uses the **Limine**'s bootloader which conveniently loads the kernel in long mode with an Higher-Half Direct Mapping (HHDM) to easily access the physical RAM.

The core kernel's code is already written as **architecture independent** meaning that Learnix can be ported on other hardware architectures as well (ARMv8 and RISC-V are next).

Learnix is designed to swap and test different components at compile time, for now:
- the physical memory manager
- the kernel keap
- the scheduler

## Subsystems
Here you'll find detailed documentation about individual subsystems:
- [mm/ - Memory Management](../kernel/mm/README.md)
- [proc/ - Processes](../kernel/proc/README.md)
- [sched/ - Scheduler](../kernel/sched/README.md)

## References
- [Operating Systems: Three Easy Pieces](https://ostep.org) - Arpaci, Dusseau
- [Understanding the Linux Kernel 3rd EDition](https://www.cs.utexas.edu/~rossbach/cs380p/papers/ulk3.pdf) - Daniel P. Bovet, Marco Cesati
- [OSDev Wiki](https://wiki.osdev.org)