#include "common.h"

void assert(char *file, int line, const char *func, bool condition, char *message);
#define ASSERT(condition, message) assert(__FILE__, __LINE__, __func__, (condition), (message))
#define TODO(message) ASSERT(false, "TODO: "message)

// Linker symbols
extern int __boot_start[];
extern int __boot_end[];
extern int __kernel_phys_start[];
extern int __kernel_phys_end[];
extern int __kernel_start[];
extern int __kernel_end[];

#include "boot/multiboot.c"
#include "platform/x86.c"
#include "hal/hal.c"
#include "std/std.c"
#include "page_frame_allocator.c"


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

void kernel_init(Multiboot_Info *boot_info) {
	terminal_init();
    bool initted = serial_init();
    ASSERT(initted, "Serial initialization failed.");
    serial_write('\n'); // @Hack: Qemu doesn't put a newline in the serial.
    interrupts_init();
    initted = pfa_init(boot_info);
    ASSERT(initted, "Page frame allocation initialization failed.");
    initted = memory_init();
    ASSERT(initted, "Memory initialization failed.");
}

void kernel_main(Multiboot_Info *boot_info) {
    kernel_init(boot_info);

    MMap_Segment *segment = (MMap_Segment *)boot_info->mmap_addr;
    size_t bytes_traversed = 0;
    while (bytes_traversed < boot_info->mmap_length) {
        fmt_print("addr: %lx, length: %lx, type: %u, size: %u\n",
            segment->base_addr, segment->length, segment->type, segment->size);

        bytes_traversed += segment->size + 4;
        segment = (MMap_Segment*)((uint8_t*)segment + segment->size + 4);
    }

    fmt_print("\n");
    fmt_print("boot_start: %x\n", (uint32_t)__boot_start);
    fmt_print("boot_end: %x\n", (uint32_t)__boot_end);
    fmt_print("kernel_phys_start: %x\n", (uint32_t)__kernel_phys_start);
    fmt_print("kernel_phys_end: %x\n", (uint32_t)__kernel_phys_end);
    fmt_print("kernel_start: %x\n", (uint32_t)__kernel_start);
    fmt_print("kernel_end: %x\n", (uint32_t)__kernel_end);

    int *some_memory = memory_allocate(sizeof(*some_memory));
    int *some_more_memory = memory_allocate(sizeof(*some_more_memory));
    *some_memory = 5;
    *some_more_memory = 6;
    fmt_print("%d, %d\n", *some_memory, *some_more_memory);
    memory_free(some_memory);
    memory_free(some_more_memory);
    
    // @FIXME: Assert that the multiboot magic number is correct.
    // ASSERT(boot_info->flags & (1 << 6), "Boot info mmap is not valid.");

    for(;;);
}
