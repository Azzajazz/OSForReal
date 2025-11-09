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
    gdt_init();
    // @Cleanup: Name
	terminal_initialize();
    bool initted = serial_init();
    ASSERT(initted, "Serial initialization failed!");

    fmt_print("\n");
    fmt_print("%s\n", boot_info->boot_loader_name);
}
