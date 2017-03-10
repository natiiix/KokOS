#!/bin/bash

set -e

./clean.sh
./prepdir.sh
./build.sh

if grub-file --is-x86-multiboot ../bin/kokos.bin; then
	echo multiboot confirmed	
	echo copying OS binary and GRUB config
	cp ../bin/kokos.bin ../isodir/boot/kokos.bin
	cp grub.cfg ../isodir/boot/grub/grub.cfg
	echo building iso
	grub-mkrescue -o ../iso/kokos.iso ../isodir/
	#qemu-system-i386 -cdrom ../iso/kokos.iso
else
	echo the file is not multiboot
fi
