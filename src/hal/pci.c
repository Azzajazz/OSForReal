#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_ADDRESS_ENABLE 0x80000000u

uint32_t pci_read_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg) {
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

    out_32(PCI_CONFIG_ADDRESS, address);
    return in_32(PCI_CONFIG_DATA);
}
