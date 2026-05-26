#!/usr/bin/env bash

set -xe

extra_flags=""
binary="os-for-real"

if [[ $OS_MODE = "debug" ]]
then
    extra_flags="-s -S -M smm=off -d int -D ./qemu.log"
fi

if [[ $OS_TEST = 1 ]]
then
    binary="os-for-real-test"
fi

# Set up the hard drive.
boot_partition_size=262144
hard_disk_size=1048576

mkdir -p devices
touch ./devices/hard_drive
truncate -s $hard_disk_size ./devices/hard_drive
./custom-tools/mkpart/build/mkpart ./devices/hard_drive
./tagfs/tfsfmt/build/tfsfmt format ./devices/hard_drive -byte-offset $boot_partition_size
echo "hello" > ./devices/hello.txt
./tagfs/tfsfmt/build/tfsfmt write-files ./devices/hard_drive -byte-offset $boot_partition_size \
    -file ./devices/hello.txt

# Run in QEMU.
qemu-system-i386 -nographic \
    -kernel build/$binary \
    -drive format=raw,file=./devices/hard_drive \
    $extra_flags
