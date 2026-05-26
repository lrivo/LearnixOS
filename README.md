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

After that you must run the `fetch-deps.sh` script:
```bash
./fetch-deps.sh
```

Now you can setup the meson project with the components of your choice.
Look into `meson.option` to see what you can do.
```bash
meson setup build -Dsched=round_robin -Dpmm=bitmap
```
Or simply launch `setup.sh` for the default configuration.

If you change idea on the configuration:
```bash
meson configure build -Dsched=mlfq
```

And finally you can compile and launch in QEMU with:
```bash
ninja -C build run
```

## Credits
- https://codeberg.org/Limine/limine-c-template-x86-64
