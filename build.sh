#!/usr/bin/env bash

set -xe

TOOLCHAIN_PREFIX=./toolchain/gcc-15.2.0-cross/bin
TARGET=i686-elf

SRC_DIR=./src
TESTS_DIR=./src/tests
BUILD_DIR=./build
ISO_DIR=./isodir

CFLAGS="-std=gnu99 -ffreestanding -Wall -Wextra -Werror -masm=intel -g"

if [[ $OS_MODE = "debug" ]]
then
    echo "In debug mode"
    CFLAGS=$CFLAGS" -O0"
else
    CFLAGS=$CFLAGS" -O2"
fi

mkdir -p $BUILD_DIR

# Build and link the kernel.
$TOOLCHAIN_PREFIX/$TARGET-as -c $SRC_DIR/boot/boot.s -o $BUILD_DIR/boot.o
$TOOLCHAIN_PREFIX/$TARGET-gcc -c $SRC_DIR/boot/bootstrap.c -o $BUILD_DIR/bootstrap.o $CFLAGS
$TOOLCHAIN_PREFIX/$TARGET-gcc -c $SRC_DIR/kernel.c -o $BUILD_DIR/kernel.o $CFLAGS
$TOOLCHAIN_PREFIX/$TARGET-gcc -T link.ld -Wl,-Map=$BUILD_DIR/output.map -Wl,--gc-sections -o $BUILD_DIR/os-for-real.elf $CFLAGS -nostdlib $BUILD_DIR/boot.o $BUILD_DIR/bootstrap.o $BUILD_DIR/kernel.o -lgcc

# It's more convenient to put the debug info in a different file.
$TOOLCHAIN_PREFIX/$TARGET-objcopy --strip-debug $BUILD_DIR/os-for-real.elf $BUILD_DIR/os-for-real
$TOOLCHAIN_PREFIX/$TARGET-objcopy --only-keep-debug $BUILD_DIR/os-for-real.elf $BUILD_DIR/os-for-real.debug
$TOOLCHAIN_PREFIX/$TARGET-objcopy --add-gnu-debuglink=$BUILD_DIR/os-for-real.debug $BUILD_DIR/os-for-real

# Build and link the tests.
$TOOLCHAIN_PREFIX/$TARGET-gcc -c $SRC_DIR/boot/bootstrap.c -o $BUILD_DIR/bootstrap_test.o -DKERNEL_TEST $CFLAGS
$TOOLCHAIN_PREFIX/$TARGET-gcc -c $TESTS_DIR/runner.c -o $BUILD_DIR/tests.o $CFLAGS
$TOOLCHAIN_PREFIX/$TARGET-gcc -T link.ld -Wl,-Map=$BUILD_DIR/output_tests.map -o $BUILD_DIR/os-for-real-test.elf $CFLAGS -nostdlib $BUILD_DIR/boot.o $BUILD_DIR/bootstrap_test.o $BUILD_DIR/tests.o -lgcc

$TOOLCHAIN_PREFIX/$TARGET-objcopy --strip-debug $BUILD_DIR/os-for-real-test.elf $BUILD_DIR/os-for-real-test
$TOOLCHAIN_PREFIX/$TARGET-objcopy --only-keep-debug $BUILD_DIR/os-for-real-test.elf $BUILD_DIR/os-for-real-test.debug
$TOOLCHAIN_PREFIX/$TARGET-objcopy --add-gnu-debuglink=$BUILD_DIR/os-for-real-test.debug $BUILD_DIR/os-for-real-test


# Verify multiboot header for both os-for-real and os-for-real-test.
if grub-file --is-x86-multiboot $BUILD_DIR/os-for-real
then
    echo "os-for-real OK: Multiboot header verification successful"
else
    echo "os-for-real ERROR: Multiboot header verification unsuccessful"
fi

if grub-file --is-x86-multiboot $BUILD_DIR/os-for-real-test
then
    echo "os-for-real-test OK: Multiboot header verification successful"
else
    echo "os-for-real-test ERROR: Multiboot header verification unsuccessful"
fi

# Make the iso image
mkdir -p $ISO_DIR/boot/grub
cp $BUILD_DIR/os-for-real $ISO_DIR/boot/os-for-real
cp $BUILD_DIR/os-for-real-test $ISO_DIR/boot/os-for-real-test
cp grub.cfg $ISO_DIR/boot/grub/grub.cfg
grub-mkrescue -o $BUILD_DIR/os-for-real.iso $ISO_DIR
