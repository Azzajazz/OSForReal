typedef struct PACKED {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
} Gate_Descriptor;

typedef struct {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} Interrupt_Frame;

#define IDT_GATE_TASK 0x5
#define IDT_GATE_INT16 0x6
#define IDT_GATE_TRAP16 0x7
#define IDT_GATE_INT32 0xE
#define IDT_GATE_TRAP32 0xF

#define IDT_PRIV_1 (1 << 5)
#define IDT_PRIV_2 (2 << 5)
#define IDT_PRIV_3 (3 << 5)

#define IDT_PRESENT (1 << 7)

struct PACKED {
    uint16_t size;
    uint32_t offset;
} idtr;

typedef void (*Registered_Handler)(Interrupt_Frame *frame);

void interrupts_init();
void idt_register_handler(int vector, Registered_Handler handler);
