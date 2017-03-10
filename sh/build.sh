#!/bin/bash

set -e

echo compiling boot.s
../../barebones-toolchain/cross/x86_64/bin/i686-elf-as ../src/boot.s -o ../obj/boot.o
echo compiling C++ code
../../barebones-toolchain/cross/x86_64/bin/i686-elf-g++ -c ../src/*.cpp -std=c++11 -ffreestanding -O2 -Wall -Wextra
echo moving object files from sh/ to obj/
mv *.o ../obj/
echo linking objects
../../barebones-toolchain/cross/x86_64/bin/i686-elf-g++ -T ../src/linker.ld -o ../bin/kokos.bin -ffreestanding -O2 -nostdlib ../obj/*.o -lgcc
