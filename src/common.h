#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)
#define PACKED __attribute__((packed))
#define INTERRUPT __attribute__((target("general-regs-only"),interrupt))

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))
