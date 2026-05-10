#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_ADDRESS_ENABLE 0x80000000u

bool pci_init() {
    // @TODO: Enumerate devices. That's quite the job, so I'm just hard-coding this for now.
    uint8_t ide_bus = 0;
    uint8_t ide_device = 1;
    uint8_t ide_function = 1;
    uint32_t fields = pci_read_register(ide_bus, ide_device, ide_function, 8);
    uint8_t class_code = (fields >> 24) & 0xFF;
    uint8_t subclass = (fields >> 16) & 0xFF;
    ASSERT(class_code == 1 && subclass == 1, "The device is not an IDE device.");

    uint8_t prog_if = (fields >> 8) & 0xFF;
    // Switch to PCI native mode if possible.
    bool should_write_prog_if = false;
    if (prog_if & 0x2) {
        prog_if |= 0x1;
        fields |= prog_if << 8; // Set bit 0 of Prog IF
        should_write_prog_if = true;
    }
    if (prog_if & 0x8) {
        prog_if |= 0x4;
        fields |= prog_if << 8; // Set bit 0 of Prog IF
        should_write_prog_if = true;
    }
    bool primary_pci_native = ((prog_if & 0x1) != 0);
    bool secondary_pci_native = ((prog_if & 0x4) != 0);
    if (should_write_prog_if) {
        pci_write_register(ide_bus, ide_device, ide_function, 8, fields);
    }

    fields = pci_read_register(ide_bus, ide_device, ide_function, 12);
    uint8_t header_type = (fields >> 24) & 0xFF;
    ASSERT(header_type == 0, "Unexpected header type.");
    uint32_t bar0 = pci_read_register(ide_bus, ide_device, ide_function, 16);
    uint32_t bar1 = pci_read_register(ide_bus, ide_device, ide_function, 20);
    uint32_t bar2 = pci_read_register(ide_bus, ide_device, ide_function, 24);
    uint32_t bar3 = pci_read_register(ide_bus, ide_device, ide_function, 28);
    uint32_t bar4 = pci_read_register(ide_bus, ide_device, ide_function, 32);

    bool initted = ide_init(primary_pci_native, secondary_pci_native, bar0, bar1, bar2, bar3, bar4);
    return initted;
}

uint32_t pci_get_register_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg) {
    ASSERT((reg & 0x3) == 0, "reg does not point to a 32-bit aligned region.");

    uint32_t bus_32 = (uint32_t)bus;
    uint32_t device_32 = (uint32_t)device;
    uint32_t function_32 = (uint32_t)function;
    uint32_t reg_32 = (uint32_t)reg;
    uint32_t address = PCI_ADDRESS_ENABLE |
        (bus_32 << 16) |
        (device_32 << 11) |
        (function_32 << 8) |
        reg_32;
    return address;
}

uint32_t pci_read_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg) {
    uint32_t address = pci_get_register_address(bus, device, function, reg);
    out_32(PCI_CONFIG_ADDRESS, address);
    return in_32(PCI_CONFIG_DATA);
}

void pci_write_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg, uint32_t value) {
    uint32_t address = pci_get_register_address(bus, device, function, reg);
    out_32(PCI_CONFIG_ADDRESS, address);
    out_32(PCI_CONFIG_DATA, value);
}
