#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)
#define PACKED __attribute__((packed))
#define INTERRUPT __attribute__((target("general-regs-only"),interrupt))

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

void assert(char *file, int line, const char *func, bool condition, char *message);
#define ASSERT(condition, message) assert(__FILE__, __LINE__, __func__, (condition), (message))

#include "boot/multiboot.c"
#include "platform/x86.c"
#include "hal/hal.c"
#include "std/std.c"

// Linker symbols
extern int __boot_start;
extern int __boot_end;
extern int __kernel_start;
extern int __kernel_end;
extern int __loaded_size;

void assert(char *file, int line, const char *func, bool condition, char *message) {
    if (condition) {
        return;
    }

    fmt_print("ASSERTION FAILED\n");
    fmt_print("%s:%s:%d: %s\n", file, func, line, message);
    cpu_halt();
}

void keyboard_handler(Interrupt_Frame *frame) {
    UNUSED(frame);
    fmt_print("Key press!");
}

void kernel_main(Multiboot_Info *boot_info)
{
    // @Cleanup: Name
	terminal_initialize();
    bool initted = serial_init();
    ASSERT(initted, "Serial initialization failed.");
    serial_write('\n'); // @Hack: Qemu doesn't put a newline in the serial.

    // @FIXME: Put all this into interrupts_init()
    pic_init();
    idt_init();

    MMap_Segment *segments = (MMap_Segment*)boot_info->mmap_addr;
    MMap_Segment *past_last_segment = (MMap_Segment*)(boot_info->mmap_addr + boot_info->mmap_length);
    while (segments < past_last_segment) {
        fmt_print("addr: %lx, length: %lu, type: %u, size: %u\n",
            segments->base_addr, segments->length, segments->type, segments->size);
        segments = (MMap_Segment*)((uint8_t*)segments + segments->size + 4);
    }
    fmt_print("\n");
    fmt_print("boot_start: %x\n", (uint32_t)&__boot_start);
    fmt_print("boot_end: %x\n", (uint32_t)&__boot_end);
    fmt_print("kernel_start: %x\n", (uint32_t)&__kernel_start);
    fmt_print("kernel_end: %x\n", (uint32_t)&__kernel_end);
    fmt_print("loaded_size: %d\n", (uint32_t)&__loaded_size);

    // @FIXME: Assert that the multiboot magic number is correct.
    ASSERT(boot_info->flags & (1 << 6), "Boot info mmap is not valid.");

    for(;;);
}
