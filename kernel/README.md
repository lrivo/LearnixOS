# Kernel

- `arch/`: hardware architecture specific code 
- `core/`
- `drivers/`: keyboard, mouse, NIC, ...
- `include/`: generic header files
- `lib/`: libc-like utility functions (`memcpy()`, `strlen()`)
- `mm/`: architecture independent memory management
- `proc/`: everything about processes
- `sched/`: schedulers

### Notes
Currently the PS/2 keyboard driver (`drivers/input/ps2kb.c`) is an architecture specific driver in a non architecture specific folder.
For now I am leaving it this way but a refactor is needed later on because I don't know exactly what do do now.
