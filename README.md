# LearnixOS SSRI

This is my Bachelor's thesis project: designing and developing a modular x86_64 kernel.

Please read the documentation [here](docs/README.md).

## Build
Make sure to have installed:
- `clang`
- `ld.lld`
- `meson`
- `ninja`
- `xorriso`
- `qemu`

Then you can fetch dependencies and let meson generate the build files:
```bash
./fetch-deps.sh
./setup.sh
```

And finally you can compile with:
```bash
nina -C target run-uefi
```

## Credits
- https://codeberg.org/Limine/limine-c-template-x86-64
