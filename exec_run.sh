#!/bin/bash

set -e

./exec_clean.sh
./exec_prepdir.sh
./exec_build_bin.sh

echo booting the kernel
qemu-system-i386 -kernel bin/kokos.bin
