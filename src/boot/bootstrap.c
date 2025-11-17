#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)
#define PACKED __attribute__((packed))

// Linker constants.
extern int __boot_start;
extern int __kernel_start;
extern int __kernel_end;
extern int __kernel_phys_start;
extern int __kernel_phys_end;
extern int __loaded_size;

#include "multiboot.c"
#include "gdt.c"
#include "paging.c"


// Kernel entry point.
void kernel_main(Multiboot_Info *boot_info);


void bootstrap(Multiboot_Info *boot_info) {
    bool initted = paging_init(boot_info);
    UNUSED(initted);

    kernel_main(boot_info);
}
