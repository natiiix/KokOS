#!/bin/bash

set -e

echo cleaning old files
rm -rfv ${BASH_SOURCE%/*}/../*.o
rm -rfv ${BASH_SOURCE%/*}/../*.bin
rm -rfv ${BASH_SOURCE%/*}/../*.iso
rm -rfv ${BASH_SOURCE%/*}/../obj/
rm -rfv ${BASH_SOURCE%/*}/../bin/
rm -rfv ${BASH_SOURCE%/*}/../isodir/
rm -rfv ${BASH_SOURCE%/*}/../iso/
