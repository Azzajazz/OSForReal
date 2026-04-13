#! /bin/bash

set -xe

mkdir -p build

if [[ $1 == "wipe" ]]
then
    rm -f driver.img
    touch driver.img
    truncate -s 512000 driver.img
    ../tfsfmt/build/tfsfmt format driver.img
fi

gcc src/tagfs.c -I libfuse/include/ -I libfuse/build -L libfuse/build/lib -lfuse3 -o build/tagfs -g
