typedef struct PACKED {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t limit_high_and_flags;
    uint8_t base_high;
} Segment_Descriptor;

#define GDT_AB_ACCESSED (1 << 0)
#define GDT_AB_RW (1 << 1)
#define GDT_AB_DIR_CONFORM (1 << 2)
#define GDT_AB_EXEC (1 << 3)
#define GDT_AB_DATACODE (1 << 4)
#define GDT_AB_PRIV_1 (1 << 5)
#define GDT_AB_PRIV_2 (2 << 5)
#define GDT_AB_PRIV_3 (3 << 5)
#define GDT_AB_PRESENT (1 << 7)

#define GDT_AB_SYS_16TSS_A 0x1
#define GDT_AB_SYS_LDT 0x2
#define GDT_AB_SYS_16TSS_B 0x3
#define GDT_AB_SYS_32TSS_A 0x9
#define GDT_AB_SYS_32TSS_B 0xB

#define GDT_FLAGS_LONG (1 << 3)
#define GDT_FLAGS_SIZE32 (1 << 2)
#define GDT_FLAGS_GRAN4K (1 << 3)



struct PACKED {
    uint16_t size;
    uint32_t offset;
} gdtr;

Segment_Descriptor gdt[5];

Segment_Descriptor gdt_create_descriptor(
    uint32_t base,
    uint32_t limit,
    uint8_t access_byte,
    uint8_t flags
) {
    Segment_Descriptor descriptor;

    descriptor.limit_low = limit & 0xffff;
    descriptor.base_low = base & 0xffff;
    descriptor.base_mid = (base >> 16) & 0xff;
    descriptor.access_byte = access_byte;
    descriptor.limit_high_and_flags = ((limit >> 16) & 0xf) | (flags << 4);
    descriptor.base_high = (base >> 24) & 0xff;

    return descriptor;
}

void gdt_init(void) {
    // Null descriptor.
    gdt[0] = (Segment_Descriptor){0};

    // Kernel code descriptor.
    gdt[1] = gdt_create_descriptor(
        0,
        0xfffff,
        GDT_AB_ACCESSED | GDT_AB_RW | GDT_AB_DIR_CONFORM | GDT_AB_EXEC
            | GDT_AB_DATACODE | GDT_AB_PRESENT,
        GDT_FLAGS_GRAN4K | GDT_FLAGS_SIZE32
    );

    // Kernel data descriptor.
    gdt[2] = gdt_create_descriptor(
        0,
        0xfffff,
        GDT_AB_ACCESSED | GDT_AB_RW | GDT_AB_DATACODE | GDT_AB_PRESENT,
        GDT_FLAGS_GRAN4K | GDT_FLAGS_SIZE32
    );

    // User code descriptor.
    gdt[3] = gdt_create_descriptor(
        0,
        0xfffff,
        GDT_AB_ACCESSED | GDT_AB_RW | GDT_AB_DIR_CONFORM | GDT_AB_EXEC
            | GDT_AB_PRIV_3 | GDT_AB_DATACODE | GDT_AB_PRESENT,
        GDT_FLAGS_GRAN4K | GDT_FLAGS_SIZE32
    );

    // User data descriptor.
    gdt[4] = gdt_create_descriptor(
        0,
        0xfffff,
        GDT_AB_ACCESSED | GDT_AB_RW | GDT_AB_PRIV_3 | GDT_AB_DATACODE | GDT_AB_PRESENT,
        GDT_FLAGS_GRAN4K | GDT_FLAGS_SIZE32
    );

    //@FIXME: TSS?

    gdtr.size = sizeof(gdt) - 1;
    gdtr.offset = (uint32_t)gdt;
    
    asm (
        "lgdt [%0]\n\t"
        "jmp 0x8:.protected\n\t"

        ".protected:\n\t"
        "mov ax, 0x10\n\t"
        "mov ds, ax\n\t"
        "mov es, ax\n\t"
        "mov fs, ax\n\t"
        "mov gs, ax\n\t"
        "mov ss, ax\n\t"
        :
        : "m" (gdtr)
    );
}
