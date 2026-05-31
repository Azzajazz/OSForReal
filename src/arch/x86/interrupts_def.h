typedef struct PACKED {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
} Gate_Descriptor;

typedef struct {
    // These fields are saved by the interrupt stub.
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // Fields below here are pushed by the CPU on interrupt.

    // Set to 0 if no error code.
    uint16_t error_code;
    uint16_t __error_code_high;
    uint32_t eip;
    uint16_t cs;
    uint16_t __cs_high;
    uint32_t eflags;

    // These are defined only for interrupts occuring while in user space.
    uint32_t esp;
    uint16_t ss;
    uint16_t __ss_high;
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
void interrupts_register_handler(int vector, Registered_Handler handler);
