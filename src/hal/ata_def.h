typedef enum {
    IDE_BUS_PRIM,
    IDE_BUS_SND,
} IDE_Bus_ID;

typedef struct {
    uint16_t io_base;
    uint16_t control_base;
    uint16_t bus_master;
} IDE_Channel_Info;

typedef enum {
    DRIVE_TYPE_ATA,
    // @TODO: Need SATA, ATAPI, etc.
    DRIVE_TYPE_UNKNOWN,
} IDE_Drive_Type;

typedef struct {
    bool missing;
    IDE_Drive_Type type;
    uint8_t udma_mode;
    uint32_t lba28_addressable_sector_count; // 0 if lba28 is not supported.
    uint64_t lba48_addressable_sector_count; // 0 if lba48 is not supported.
} IDE_Drive_Info;

bool ide_init(bool primary_pci_native, bool secondary_pci_native, uint16_t bar0, uint16_t bar1, uint16_t bar2, uint16_t bar3, uint16_t bar4);
void ata_read(IDE_Bus_ID bus, uint8_t drive, uint8_t sector_count, uint32_t lba28, void *buffer, size_t buffer_length);
