#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)

#include "hal/hal.c"
#include "std/std.c"

void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();

	terminal_fmt_print_impl("Line %c %u %d %d\n", 'a', 4294967295u, 500, -500);
}
