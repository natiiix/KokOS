#!/bin/bash

set -e

${BASH_SOURCE%/*}/clean.sh
${BASH_SOURCE%/*}/prepdir.sh

echo compiling boot.s
${BASH_SOURCE%/*}/../crosscompiler/bin/i686-elf-as ${BASH_SOURCE%/*}/../src/boot.s -o ${BASH_SOURCE%/*}/../obj/boot.o
echo compiling C++ code
${BASH_SOURCE%/*}/../crosscompiler/bin/i686-elf-g++ -c ${BASH_SOURCE%/*}/../src/*.cpp -std=c++11 -ffreestanding -O2 -Wall -Wextra
echo moving object files from active directory to obj/
mv *.o ${BASH_SOURCE%/*}/../obj/
echo linking objects
${BASH_SOURCE%/*}/../crosscompiler/bin/i686-elf-g++ -T ${BASH_SOURCE%/*}/../src/linker.ld -o ${BASH_SOURCE%/*}/../bin/kokos.bin -ffreestanding -O2 -nostdlib ${BASH_SOURCE%/*}/../obj/*.o -lgcc
echo build successful
