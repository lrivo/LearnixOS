#!/bin/bash

rm -rf deps
mkdir deps
cd deps

# edk2-ovmf
curl -L https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/edk2-ovmf.tar.gz | gunzip | tar -xf -

# fetch and compile the Limine's bootloader
rm -rf limine
git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1
make -C limine CC=clang

# limine protocol headers
git clone https://codeberg.org/Limine/limine-protocol.git limine-protocol 
git -C limine-protocol/ checkout 068b6481557db836e41bf644382367f8dee76d21

# freestnd-c-hdrs
git clone https://codeberg.org/OSDev/freestnd-c-hdrs-0bsd.git freestnd-c-hdrs
git -C freestnd-c-hdrs/ checkout 097259a899d30f0a4b7a694de2de5fdda942e923

# cc-runtime
git clone https://codeberg.org/OSDev/cc-runtime.git cc-runtime
git -C cc-runtime checkout dae79833b57a01b9fd3e359ee31def69f5ae899b
