#!/bin/bash

set -e

echo Preparing directory tree
mkdir ${BASH_SOURCE%/*}/../obj/
mkdir ${BASH_SOURCE%/*}/../obj/c/
mkdir ${BASH_SOURCE%/*}/../obj/cpp/
mkdir ${BASH_SOURCE%/*}/../obj/nasm/
mkdir ${BASH_SOURCE%/*}/../bin/
mkdir ${BASH_SOURCE%/*}/../isodir/
mkdir ${BASH_SOURCE%/*}/../isodir/boot/
mkdir ${BASH_SOURCE%/*}/../isodir/boot/grub/
mkdir ${BASH_SOURCE%/*}/../iso/
