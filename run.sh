#!/usr/bin/env bash

set -xe

if [[ $OS_MODE = "debug" ]]
then
    qemu-system-i386 -nographic -kernel build/os-for-real.bin -s -S
else
    qemu-system-i386 -nographic -kernel build/os-for-real.bin
fi
