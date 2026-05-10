bool pci_init();
uint32_t pci_read_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg);
void pci_write_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg, uint32_t value);
