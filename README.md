# OSForReal

A hobby OS. I'm trying this for the third time, but skipping all the bootloader stuff since it's a pain.

## Building

Building OSForReal requires an i386 `gcc` v15.2.0 cross compiler. Follow the steps in the [OSDev Wiki](https://wiki.osdev.org/GCC_Cross-Compiler) to build your own.
Install the cross compiler at `/path/to/OSForReal/toolchain/gcc-15.2.0-cross`

Once you have a cross compiler at the correct location, build OSForReal by running one of the following commands:
- Release build: `./build.sh`
- Debug build:  `OS_MODE=debug ./build.sh`

## Running

OSForReal has only been tested on `qemu-system-i386`. You are welcome to configure the QEMU emulator however you like, but be aware that OSForReal will not make use of all features that the emulator provides. To use the recommended configuration, run the following commands:
- Run normally: `./run.sh`
- Run with some debugging capabilities: `OS_MODE=debug ./run.sh`

## Debugging

QEMU exposes a `gdb` stub on `localhost:1234`. If your debugger supports connecting to a remote `gdb` server, then you can debug the kernel as follows:
1. Build and run the kernel in debug mode.
```
export OS_MODE=debug
./build.sh && ./run.sh
```
2. Connect your debugger to `localhost:1234`.
