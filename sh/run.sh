#!/bin/bash

# using ${BASH_SOURCE%/*} instead of . to make sure the path is considered relative
# to the currently executed bash script rather than the bash active directory

set -e

${BASH_SOURCE%/*}/build.sh

echo booting the kernel
qemu-system-i386 -kernel ${BASH_SOURCE%/*}/../bin/kokos.bin
