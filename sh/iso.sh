#!/bin/bash

set -e

${BASH_SOURCE%/*}/build.sh

if grub-file --is-x86-multiboot ${BASH_SOURCE%/*}/../bin/kokos.bin; then
	echo Multiboot confirmed
	echo Copying OS binary and GRUB config
	cp ${BASH_SOURCE%/*}/../bin/kokos.bin ${BASH_SOURCE%/*}/../isodir/boot/kokos.bin
	cp ${BASH_SOURCE%/*}/../src/core/grub.cfg ${BASH_SOURCE%/*}/../isodir/boot/grub/grub.cfg
	echo Building iso
	grub-mkrescue -o ${BASH_SOURCE%/*}/../iso/kokos.iso ${BASH_SOURCE%/*}/../isodir/
	qemu-system-i386 -cdrom ${BASH_SOURCE%/*}/../iso/kokos.iso -boot d
else
	echo The file is not multiboot
fi
