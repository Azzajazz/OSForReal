#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)

#include "platform/x86.c"
#include "hal/hal.c"
#include "std/std.c"

void kernel_main(void) 
{
	terminal_initialize();
    serial_init();

	fmt_print("Line %c %u %d %d %x\n", 'a', 4294967295u, 500, -500, 0x5a, 100);
}
