void out_8(uint16_t port, uint8_t data) {
    asm volatile (
        "out dx, al"
        :
        : "d" (port), "a" (data)
    );
}

uint8_t in_8(uint16_t port) {
    uint8_t data;
    asm volatile  (
        "in al, dx"
        : "=a" (data)
        : "d" (port)
    );
    return data;
}

void out_16(uint16_t port, uint16_t data) {
    asm volatile (
        "out dx, ax"
        :
        : "d" (port), "a" (data)
    );
}

uint16_t in_16(uint16_t port) {
    uint16_t data;
    asm volatile  (
        "in ax, dx"
        : "=a" (data)
        : "d" (port)
    );
    return data;
}

void out_32(uint16_t port, uint32_t data) {
    asm volatile (
        "out dx, eax"
        :
        : "d" (port), "a" (data)
    );
}

uint32_t in_32(uint16_t port) {
    uint32_t data;
    asm volatile (
        "in eax, dx"
        : "=a" (data)
        : "d" (port)
    );
    return data;
}

void cpu_halt() {
    asm volatile (
        "cli\n\t"
        "1: hlt\n\t"
        "jmp 1b\n\t"
    );
}
