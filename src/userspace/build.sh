TOOLCHAIN_PREFIX=../../toolchain/gcc-15.2.0-cross/bin
TARGET=i686-elf

SRC_DIR=.
BUILD_DIR=./build

CFLAGS="-std=gnu99 -ffreestanding -Wall -Wextra -Werror -masm=intel"

mkdir -p $BUILD_DIR
$TOOLCHAIN_PREFIX/$TARGET-gcc -T link.ld -Wl,-Map=$BUILD_DIR/userspace.map -Wl,--gc-sections $SRC_DIR/userspace.c -o $BUILD_DIR/userspace $CFLAGS -nostdlib
