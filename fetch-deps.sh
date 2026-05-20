#!/bin/bash

# edk2-ovmf
curl -L https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/edk2-ovmf.tar.gz | gunzip | tar -xf -

# limine
rm -rf limine
git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1
make -C limine CC=clang

# kernel deps
bash kernel/get-deps.sh
