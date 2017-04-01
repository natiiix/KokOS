#!/bin/bash

set -e

${BASH_SOURCE%/*}/clean.sh
${BASH_SOURCE%/*}/prepdir.sh

GCC_RUN="${BASH_SOURCE%/*}/../crosscompiler/bin/i686-elf-gcc"
GPP_RUN="${BASH_SOURCE%/*}/../crosscompiler/bin/i686-elf-g++"

GCC_PARAMS="-std=gnu99 -ffreestanding -O2 -Wall -Wextra"
GPP_PARAMS="-std=c++11 -ffreestanding -O2 -Wall -Wextra"

DIR_SOURCE="${BASH_SOURCE%/*}/../src"
DIR_INCLUDE="${BASH_SOURCE%/*}/../include"
DIR_OBJECTS="${BASH_SOURCE%/*}/../obj"

# compileNasm <directory> <filename without extension>
function compileNasm()
{
    nasm -f elf32 -g -F dwarf $DIR_SOURCE/$1$2.asm -o $DIR_OBJECTS/nasm/$2.o
}

# compileC <directory>
function compileC()
{
    $GCC_RUN -I $DIR_INCLUDE -c $DIR_SOURCE/$1/*.c $GCC_PARAMS
}

# compileCpp <directory>
function compileCpp()
{
    $GPP_RUN -I $DIR_INCLUDE -c $DIR_SOURCE/$1/*.cpp $GPP_PARAMS
}

echo compiling NASM code
compileNasm core/ boot
compileNasm core/ lowlevel

echo compiling C code
compileC kernel
compileC c
compileC drivers
compileC drivers/io
compileC drivers/storage

echo moving C object files from active directory to obj/c/
mv *.o $DIR_OBJECTS/c/

echo compiling C++ code
compileCpp cpp
compileCpp modules

echo moving C++ object files from active directory to obj/cpp/
mv *.o $DIR_OBJECTS/cpp/

echo linking objects
$GPP_RUN -T $DIR_SOURCE/core/link.ld -o ${BASH_SOURCE%/*}/../bin/kokos.bin -ffreestanding -O2 -nostdlib $DIR_OBJECTS/nasm/*.o $DIR_OBJECTS/c/*.o $DIR_OBJECTS/cpp/*.o -lgcc

echo build successful
