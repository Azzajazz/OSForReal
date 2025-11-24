#include "../kernel.c"

void assert(char *file, int line, const char *func, bool condition, char *message);
#define ASSERT(condition, message) assert(__FILE__, __LINE__, __func__, (condition), (message))

void kernel_test(Multiboot_Info *boot_info) {
    UNUSED(boot_info);
    kernel_init();

    fmt_print("Hello from tests!");
    for (;;);
}
