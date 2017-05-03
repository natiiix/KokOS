#!/bin/bash

set -e

# Download and install dependencies
sudo apt-get update -y
sudo apt-get dist-upgrade -y
sudo apt-get check -y
sudo apt-get autoremove -y
sudo apt-get install libgmp3-dev libmpfr-dev libisl-dev libcloog-isl-dev libmpc-dev texinfo -y

# Install other tools required for building the OS
sudo apt-get install nasm -y
sudo apt-get install xorriso -y

cd .. #/
rm -rfv crosscompiler/
mkdir crosscompiler/
cd crosscompiler/ #/crosscompiler/

# Set installation variables
export BINUTILS_VERSION=2.28
export GCC_VERSION=6.3.0

export PREFIX="$PWD/"
export TARGET="i686-elf"
export PATH="$PREFIX/bin:$PATH"

# Make a folder for temporary files that will be deleted after the installation is done
mkdir tmpdir/
cd tmpdir/ #/crosscompiler/tmpdir/

# Download source code archives
wget ftp://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
wget ftp://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz

# Unzip the source code
tar -xvzf binutils-$BINUTILS_VERSION.tar.gz
tar -xvzf gcc-$GCC_VERSION.tar.gz

# Build binutils
mkdir build-binutils/
cd build-binutils/ #/crosscompiler/tmpdir/build-binutils/
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix=$PREFIX --disable-nls --disable-multilib --disable-werror
make
make install
cd .. #/crosscompiler/tmpdir/

# Build gcc
mkdir build-gcc/
cd build-gcc/ #/crosscompiler/tmpdir/build-gcc/
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix=$PREFIX --disable-nls --disable-multilib --enable-languages=c,c++
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
cd .. #/crosscompiler/tmpdir/

cd .. #/crosscompiler/

# Delete the temporary folder with all the unnecessary files
rm -rfv tmpdir/
