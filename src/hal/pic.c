// Ports
#define PICM_COMMAND 0x20
#define PICM_DATA 0x21
#define PICS_COMMAND 0x40
#define PICS_DATA 0x41

// Initialisation Command Words.
#define PIC_ICW1_ICW4 (1 << 0)
#define PIC_ICw1_SINGLE (1 << 1)
#define PIC_ICW1_ADDR_INT (1 << 2)
#define PIC_ICW1_LVL_TRIG (1 << 3)
#define PIC_ICW1_INIT (1 << 4) // REQUIRED!

#define PIC_ICW3_S_IRQ (1 << 2) // Only on x86.
#define PIC_ICW3_S_ID 2 // Only on x86.

#define PIC_ICW4_8086 (1 << 0)
#define PIC_ICW4_AUTO_EOI (1 << 1)
#define PIC_ICW4_BUF_S (2 << 2)
#define PIC_ICW4_BUF_M (3 << 2)
#define PIC_ICW4_SFNM (1 << 4) // Special fully nested mode.

// Operation Command Words.
#define PIC_OCW2_EOI 0x20

void pic_init() {
    // Master configuration.
    out_8(PICM_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    out_8(PICM_DATA, 0x20);
    out_8(PICM_DATA, PIC_ICW3_S_IRQ);
    out_8(PICM_DATA, PIC_ICW4_8086 | PIC_ICW4_AUTO_EOI);

    // Slave configuration.
    out_8(PICS_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    out_8(PICS_DATA, 0x30);
    out_8(PICS_DATA, PIC_ICW3_S_ID);
    out_8(PICS_DATA, PIC_ICW4_8086 | PIC_ICW4_AUTO_EOI);

    // Unmask the PICs.
    out_8(PICM_DATA, 0);
    out_8(PICS_DATA, 0);
}
