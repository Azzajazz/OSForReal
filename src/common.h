#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

// Meaningful abbreviations for standard statements.
#define UNUSED(x) (void)(x)

// Attribute abbreviations.
#define PACKED __attribute__((packed))
#define INTERRUPT __attribute__((target("general-regs-only"),interrupt))
#define ALIGN(x) __attribute__((aligned(x)))

// Helpful macros.
#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

// Standard units.
#define KiB 1024
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)

// Shared constants.
#define PAGE_SIZE 4096
#define MAX_KERNEL_SIZE 8 * MiB

// Shared types.
typedef struct {uint32_t value;} Phys_Addr; // Strongly typed physical address
