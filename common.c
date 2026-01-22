#define ARRAY_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define PACKED __attribute__((packed))
#define CEIL_DIV(m, n) (((m) + (n) - 1) / (n))

