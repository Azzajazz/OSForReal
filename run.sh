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

qemu-system-i386 -nographic -kernel build/$binary $extra_flags
