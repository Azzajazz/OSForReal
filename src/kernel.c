#include "common.h"
#include "forward.h"

void assert(char *file, int line, const char *func, bool condition, char *message);
#define ASSERT(condition, message) assert(__FILE__, __LINE__, __func__, (condition), (message))
#define TODO(message) ASSERT(false, "TODO: "message)

// Linker symbols
extern char __boot_start[];
extern char __boot_end[];
extern char __kernel_phys_start[];
extern char __kernel_phys_end[];
extern char __kernel_start[];
extern char __kernel_end[];

#include "platform/x86.c"
#include "hal/hal.c"
#include "std/std.c"
#include "page_frame_allocator.c"


void assert(char *file, int line, const char *func, bool condition, char *message) {
    if (condition) {
        return;
    }

    fmt_print("ASSERTION FAILED\n");
    fmt_print("%s:%s:%d: %s\n", file, func, line, message);
    cpu_halt();
}

void keyboard_handler(Interrupt_Frame *frame) {
    UNUSED(frame);
    fmt_print("Key press!");
}

void kernel_init(Multiboot_Info *boot_info) {
	terminal_init();
    bool initted = serial_init();
    ASSERT(initted, "Serial initialization failed.");
    serial_write('\n'); // @Hack: Qemu doesn't put a newline in the serial.
    interrupts_init();
    initted = pfa_init(boot_info);
    ASSERT(initted, "Page frame allocation initialization failed.");
    initted = memory_init();
    ASSERT(initted, "Memory initialization failed.");
    initted = ata_init();
    ASSERT(initted, "ATA initialization failed.");
}

void kernel_main(Multiboot_Info *boot_info) {
    kernel_init(boot_info);

    MMap_Segment *segment = (MMap_Segment *)boot_info->mmap_addr;
    size_t bytes_traversed = 0;
    while (bytes_traversed < boot_info->mmap_length) {
        fmt_print("addr: %lx, length: %lx, type: %u, size: %u\n",
            segment->base_addr, segment->length, segment->type, segment->size);

        bytes_traversed += segment->size + 4;
        segment = (MMap_Segment*)((uint8_t*)segment + segment->size + 4);
    }

    fmt_print("\n");
    fmt_print("boot_start: %x\n", (uint32_t)__boot_start);
    fmt_print("boot_end: %x\n", (uint32_t)__boot_end);
    fmt_print("kernel_phys_start: %x\n", (uint32_t)__kernel_phys_start);
    fmt_print("kernel_phys_end: %x\n", (uint32_t)__kernel_phys_end);
    fmt_print("kernel_start: %x\n", (uint32_t)__kernel_start);
    fmt_print("kernel_end: %x\n", (uint32_t)__kernel_end);

    fmt_print("\n");
    uint16_t bus = 0;
    uint16_t device = 1;
    uint16_t function = 1;
    uint32_t header0 = pci_read_register(bus, device, function, 0);
    uint16_t vendor_id = (uint16_t)header0;
    uint16_t device_id = header0 >> 16;
    fmt_print("bus: %hu, device: %hhu\n", bus, device);
    fmt_print("vendor_id: %hx, device_id: %hx\n", vendor_id, device_id);
    uint32_t header2 = pci_read_register(bus, device, function, 8);
    uint8_t class_code = (header2 >> 24) & 0xFF;
    uint8_t subclass = (header2 >> 16) & 0xFF;
    uint8_t prog_if = (header2 >> 8) & 0xFF;
    uint8_t revision_id = header2 & 0xFF;
    fmt_print("class_code: %hhx, subclass: %hhx\n", class_code, subclass);
    fmt_print("prog_if: %hhx, revision_id: %hhx\n", prog_if, revision_id);
    uint32_t header3 = pci_read_register(bus, device, function, 12);
    uint8_t bist = (header3 >> 24) & 0xFF;
    uint8_t header_type = (header3 >> 16) & 0xFF;
    uint8_t latency_timer = (header3 >> 8) & 0xFF;
    uint8_t cache_line = header3 & 0xFF;
    fmt_print("bist: %hhx, header_type: %hhx\n", bist, header_type);
    fmt_print("latency_timer: %hhx, cache_line: %hhx\n", latency_timer, cache_line);
    uint32_t bar0 = pci_read_register(bus, device, function, 16);
    uint32_t bar1 = pci_read_register(bus, device, function, 20);
    uint32_t bar2 = pci_read_register(bus, device, function, 24);
    uint32_t bar3 = pci_read_register(bus, device, function, 28);
    uint32_t bar4 = pci_read_register(bus, device, function, 32);
    fmt_print("bar0: %x\n", bar0);
    fmt_print("bar1: %x\n", bar1);
    fmt_print("bar2: %x\n", bar2);
    fmt_print("bar3: %x\n", bar3);
    fmt_print("bar4: %x\n", bar4);
    
    for(;;);
}
