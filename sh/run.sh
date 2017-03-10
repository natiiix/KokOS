#!/bin/bash

set -e

./clean.sh
./prepdir.sh
./build.sh

echo booting the kernel
qemu-system-i386 -kernel ../bin/kokos.bin
