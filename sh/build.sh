#!/bin/bash

set -e

echo compiling boot.s
../../barebones-toolchain/cross/x86_64/bin/i686-elf-as ../src/boot.s -o ../obj/boot.o
echo compiling kernel.cpp
../../barebones-toolchain/cross/x86_64/bin/i686-elf-g++ -c ../src/kernel.cpp -o ../obj/kernel.o -std=c++11 -ffreestanding -O2 -Wall -Wextra
echo linking
../../barebones-toolchain/cross/x86_64/bin/i686-elf-g++ -T ../src/linker.ld -o ../bin/kokos.bin -ffreestanding -O2 -nostdlib ../obj/boot.o ../obj/kernel.o -lgcc
