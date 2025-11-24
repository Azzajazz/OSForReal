#include <stdint.h>
#include <stdbool.h>

// @FIXME: Put these in a common defs file and share between here and the kernel.
#define UNUSED(x) (void)(x)
#define PACKED __attribute__((packed))
#define INTERRUPT __attribute__((target("general-regs-only"),interrupt))

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

#include "../src/kernel.c"

void assert(char *file, int line, const char *func, bool condition, char *message);
#define ASSERT(condition, message) assert(__FILE__, __LINE__, __func__, (condition), (message))

void kernel_test(Multiboot_Info *boot_info) {
    UNUSED(boot_info);
    // @Cleanup: Name
	terminal_initialize();
    bool initted = serial_init();
    ASSERT(initted, "Serial initialization failed.");
    serial_write('\n'); // @Hack: Qemu doesn't put a newline in the serial.

    // @FIXME: Put all this into interrupts_init()
    pic_init();
    idt_init();

    fmt_print("Hello from tests!");
    for (;;);
}
