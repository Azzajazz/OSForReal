void out_8(uint16_t port, uint8_t data) {
    asm (
        "out dx, al"
        :
        : "d" (port), "a" (data)
    );
}

uint8_t in_8(uint16_t port) {
    uint8_t data;
    asm (
        "in al, dx"
        : "=a" (data)
        : "d" (port)
    );
    return data;
}

void cpu_halt() {
    asm (
        "cli\n\t"
        "1: hlt\n\t"
        "jmp 1b\n\t"
    );
}
