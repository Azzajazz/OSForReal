#!/usr/bin/env bash

set -xe

extra_flags=""
binary="os-for-real"

if [[ $OS_MODE = "debug" ]]
then
    extra_flags="-s -S -d int -M smm=off"
fi

if [[ $OS_TEST = 1 ]]
then
    binary="os-for-real-test"
fi


# Set up the hard drive.
mkdir -p devices
touch ./devices/hard_drive
truncate -s 512000 ./devices/hard_drive
./tagfs/tfsfmt/build/tfsfmt format ./devices/hard_drive

qemu-system-i386 -nographic \
    -kernel build/$binary \
    -hda ./devices/hard_drive \
    $extra_flags
