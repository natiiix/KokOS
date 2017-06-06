#!/bin/bash

set -e

# compileNasm <directory> <filename without extension>
function compileNasm()
{
    echo NASM: $1/$2.asm
    nasm -f elf32 -g -F dwarf $DIR_SOURCE/$1/$2.asm -o $DIR_OBJECTS/nasm/$2.o
}

# compileC <directory>
function compileC()
{
    echo GCC: $1/
    $GCC_RUN -I $DIR_INCLUDE -c $DIR_SOURCE/$1/*.c $GCC_PARAMS
}

# compileCpp <directory>
function compileCpp()
{
    echo G++: $1/
    $GPP_RUN -I $DIR_INCLUDE -c $DIR_SOURCE/$1/*.cpp $GPP_PARAMS
}

# build <debug mode>
function build()
{
    COMMON_RUN="${BASH_SOURCE%/*}/../crosscompiler/bin"
    GCC_RUN="$COMMON_RUN/i686-elf-gcc"
    GPP_RUN="$COMMON_RUN/i686-elf-g++"

    COMMON_PARAMS="-ffreestanding -O2 -Wall -Wextra -Wpedantic -Werror"
    
    # Debug mode enabled
    if [ $1 = "true" ]
    then
        COMMON_PARAMS+=" -D DEBUG"
    fi

    GCC_PARAMS="-std=gnu99 $COMMON_PARAMS"
    GPP_PARAMS="-std=c++11 $COMMON_PARAMS"

    DIR_SOURCE="${BASH_SOURCE%/*}/../src"
    DIR_INCLUDE="${BASH_SOURCE%/*}/../include"
    DIR_OBJECTS="${BASH_SOURCE%/*}/../obj"

    echo Compiling NASM code
    compileNasm core boot
    compileNasm core lowlevel

    echo Compiling C code
    compileC kernel
    compileC c
    compileC drivers
    compileC drivers/io
    compileC drivers/storage
    compileC drivers/storage/fat

    echo Moving C object files to /obj/c/
    mv *.o $DIR_OBJECTS/c/

    echo Compiling C++ code
    compileCpp cpp
    compileCpp modules
    compileCpp modules/commands
    compileCpp modules/execProgram

    echo Moving C++ object files to /obj/cpp/
    mv *.o $DIR_OBJECTS/cpp/

    OUTPUT_BINARY_NAME="kokos"
    if [ $1 = "true" ]
    then
        OUTPUT_BINARY_NAME+="debug"
    fi

    echo Linking objects
    $GPP_RUN -T $DIR_SOURCE/core/link.ld -o ${BASH_SOURCE%/*}/../bin/${OUTPUT_BINARY_NAME}.bin -ffreestanding -O2 -nostdlib $DIR_OBJECTS/nasm/*.o $DIR_OBJECTS/c/*.o $DIR_OBJECTS/cpp/*.o -lgcc

    echo Build successful
}

${BASH_SOURCE%/*}/clean.sh
${BASH_SOURCE%/*}/prepdir.sh

echo Building standard version
build false

echo Building debug version
build true
