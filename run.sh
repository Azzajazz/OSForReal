#!/usr/bin/env bash

set -xe

extra_flags=""
binary="os-for-real"

if [[ $OS_MODE = "debug" ]]
then
    extra_flags="-s -S -M smm=off"
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
echo "hello" > ./devices/hello.txt
./tagfs/tfsfmt/build/tfsfmt write-files ./devices/hard_drive -file ./devices/hello.txt

qemu-system-i386 -nographic \
    -kernel build/$binary \
    -drive format=raw,file=./devices/hard_drive \
    $extra_flags
