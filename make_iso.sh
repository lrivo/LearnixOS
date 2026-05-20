#!/usr/bin/env bash
set -euo pipefail

XORRISO="$1"
LIMINE="$2"
PRIVATE_DIR="$3"
SOURCE_ROOT="$4"
OUTPUT="$5"
shift 5
# remaining args are input ELFs

ISO_DIR="$PRIVATE_DIR/iso_root"
rm -rf "$ISO_DIR"
mkdir -p "$ISO_DIR/boot/limine"
mkdir -p "$ISO_DIR/EFI/BOOT"

# Copy kernel and userspace ELFs
for input_file in "$@"; do
    filename=$(basename "$input_file")
    cp -v "$input_file" "$ISO_DIR/boot/${filename%.elf}"
done

# Copy Limine config and assets
cp -v "$SOURCE_ROOT/limine.conf"              "$ISO_DIR/boot/limine/"
cp -v "$SOURCE_ROOT/limine/limine-bios.sys"   "$ISO_DIR/boot/limine/"
cp -v "$SOURCE_ROOT/limine/limine-bios-cd.bin" "$ISO_DIR/boot/limine/"
cp -v "$SOURCE_ROOT/limine/limine-uefi-cd.bin" "$ISO_DIR/boot/limine/"
cp -v "$SOURCE_ROOT/limine/BOOTX64.EFI"       "$ISO_DIR/EFI/BOOT/"
cp -v "$SOURCE_ROOT/limine/BOOTIA32.EFI"       "$ISO_DIR/EFI/BOOT/"

# Build the ISO
"$XORRISO" -as mkisofs \
    -R -r -J \
    -b boot/limine/limine-bios-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    -hfsplus -apm-block-size 2048 \
    --efi-boot boot/limine/limine-uefi-cd.bin \
    -efi-boot-part --efi-boot-image \
    --protective-msdos-label \
    "$ISO_DIR" -o "$OUTPUT"

# Install Limine BIOS boot sectors
"$LIMINE" bios-install "$OUTPUT"
