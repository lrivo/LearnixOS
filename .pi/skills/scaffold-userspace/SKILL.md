---
name: scaffold-userspace
description: Scaffold a new userspace program for the LearnixOS kernel (meson build + Limine ISO module wiring). Use when asked to add a new userspace program, e.g. "add a program called foo".
---

# Scaffold a new userspace program

Four touch points are required. Given a program name `<name>`:

1. **Source + meson file** — create `userspace/<name>/<name>.c` (with `#include <stdio.h>` and a `main`) and `userspace/<name>/meson.build`:

```meson
<name>_elf = executable(
  '<name>.elf',
  '<name>.c',
  c_args: userspace_c_args,
  link_args: userspace_link_args,
  dependencies: liblearnix_dep,
)
```

   The `userspace_c_args` / `userspace_link_args` variables come from `userspace/meson.build` (freestanding, nostdlib, custom linker script `userspace/userspace.ld`, links against `liblearnix`).

2. **Register subdir** — append `subdir('<name>')` to `userspace/meson.build` (after the last existing `subdir`).

3. **ISO dependency** — in the root `meson.build`, add `<name>_elf` to the `iso_deps` list (the custom `iso_target` builds the bootable ISO from these ELFs).

4. **Limine module** — in `limine.conf`, add to the `/boot` entry section:

```
    module_cmdline: <name>
    module_path: boot():/boot/<name>
```

   `module_cmdline` is the argv the kernel passes to the program's init; the kernel looks up the module via `module_path` (see `kernel/lib/` Limine module lookup).

## Verify

Build to confirm it compiles and gets packaged (see `make_iso.sh` / `setup.sh` for the build flow). Check no trailing-newline warnings on edited meson files.
