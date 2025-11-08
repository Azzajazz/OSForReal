#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)
#define PACKED __attribute__((packed))

#include "platform/x86.c"
#include "boot/boot.c"
#include "hal/hal.c"
#include "std/std.c"

void kernel_main(Multiboot_Info *boot_info) 
{
    gdt_init();
    // @Cleanup: Name
	terminal_initialize();
    serial_init();

    fmt_print("\n");
    fmt_print("%s\n", boot_info->boot_loader_name);
}
