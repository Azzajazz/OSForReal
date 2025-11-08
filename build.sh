#!/usr/bin/env bash

set -xe

TOOLCHAIN_PREFIX=./toolchain/gcc-15.2.0-cross/bin
TARGET=i686-elf

SRC_DIR=./src
BUILD_DIR=./build
ISO_DIR=./isodir

CFLAGS="-std=gnu99 -ffreestanding -Wall -Wextra -Werror -masm=intel"

if [[ $OS_MODE = "debug" ]]
then
    echo "In debug mode"
    CFLAGS=$CFLAGS" -g -O0"
else
    CFLAGS=$CFLAGS" -O2"
fi


# Build and link the kernel.
mkdir -p $BUILD_DIR
$TOOLCHAIN_PREFIX/$TARGET-as $SRC_DIR/boot.s -o $BUILD_DIR/boot.o
$TOOLCHAIN_PREFIX/$TARGET-gcc -c $SRC_DIR/kernel.c -o $BUILD_DIR/kernel.o $CFLAGS
$TOOLCHAIN_PREFIX/$TARGET-gcc -T link.ld -o $BUILD_DIR/os-for-real.bin $CFLAGS -nostdlib $BUILD_DIR/boot.o $BUILD_DIR/kernel.o -lgcc

# Verify multiboot header.
if grub-file --is-x86-multiboot $BUILD_DIR/os-for-real.bin
then
    echo "OK: Multiboot header verification successful"
else
    echo "ERROR: Multiboot header verification unsuccessful"
fi

# Make the iso image
mkdir -p $ISO_DIR/boot/grub
cp $BUILD_DIR/os-for-real.bin $ISO_DIR/boot/os-for-real.bin
cp grub.cfg $ISO_DIR/boot/grub/grub.cfg
grub-mkrescue -o $BUILD_DIR/os-for-real.iso $ISO_DIR
