#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)
#define PACKED __attribute__((packed))

void assert(char *file, int line, const char *func, bool condition, char *message);
#define ASSERT(condition, message) assert(__FILE__, __LINE__, __func__, (condition), (message))

#include "platform/x86.c"
#include "boot/boot.c"
#include "hal/hal.c"
#include "std/std.c"

void assert(char *file, int line, const char *func, bool condition, char *message) {
    if (condition) {
        return;
    }

    fmt_print("ASSERTION FAILED\n");
    fmt_print("%s:%s:%d: %s\n", file, func, line, message);
    asm (
        "cli\n\t"
        "1: hlt\n\t"
        "jmp 1b\n\t"
    );
}

void kernel_main(Multiboot_Info *boot_info) 
{
    // @FIXME: Assert that the multiboot magic number is correct.
    gdt_init();
    // @Cleanup: Name
	terminal_initialize();
    bool initted = serial_init();
    ASSERT(initted, "Serial initialization failed.");
    serial_write('\n'); // @Hack: Qemu doesn't put a newline in the serial.

    ASSERT(boot_info->flags & (1 << 6), "Boot info mmap is not valid.");

    MMap_Segment *segments = (MMap_Segment*)boot_info->mmap_addr;
    MMap_Segment *past_last_segment = (MMap_Segment*)(boot_info->mmap_addr + boot_info->mmap_length);
    while (segments < past_last_segment) {
        fmt_print("addr: %lx, length: %lu, type: %u, size: %u\n",
            segments->base_addr, segments->length, segments->type, segments->size);
        segments = (MMap_Segment*)((uint8_t*)segments + segments->size + 4);
    }
}
