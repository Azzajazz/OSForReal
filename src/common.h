#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

// Meaningful abbreviations for standard statements.
#define UNUSED(x) (void)(x)

// Attribute abbreviations.
#define PACKED __attribute__((packed))
#define INTERRUPT __attribute__((target("general-regs-only"),interrupt))

// Helpful macros.
#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

// Standard units.
#define KiB 1024
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)
