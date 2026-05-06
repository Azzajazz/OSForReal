// Ports.
// @TODO: These are hardcoded because qemu has them set up that way,
// but in the real world these could be different. Look at the PCI headers
// to determine the I/O base and Command base.
#define ATA_PRIM_DATA 0x1F0
#define ATA_PRIM_ERROR 0x1F1
#define ATA_PRIM_FEATURES 0x1F1
#define ATA_PRIM_SECTOR_COUNT 0x1F2
#define ATA_PRIM_LBALOW_OR_SECTORNUM 0x1F3
#define ATA_PRIM_LBAMID_OR_CYLINDERLOW 0x1F4
#define ATA_PRIM_LBAHIGH_OR_CYLINDERHIGH 0x1F5
#define ATA_PRIM_DRIVE_OR_HEAD 0x1F6
#define ATA_PRIM_STATUS 0x1F7
#define ATA_PRIM_COMMAND 0x1F7

#define ATA_PRIM_ALT_STATUS 0x3F6
#define ATA_PRIM_DEV_CONTROL 0x3F6
#define ATA_PRIM_DRIVE_ADDRESS 0x3F7

#define ATA_SND_DATA 0x170
#define ATA_SND_ERROR 0x171
#define ATA_SND_FEATURES 0x171
#define ATA_SND_SECTOR_COUNT 0x172
#define ATA_SND_LBALOW_OR_SECTORNUM 0x173
#define ATA_SND_LBAMID_OR_CYLINDERLOW 0x174
#define ATA_SND_LBAHIGH_OR_CYLINDERHIGH 0x175
#define ATA_SND_DRIVE_OR_HEAD 0x176
#define ATA_SND_STATUS 0x177
#define ATA_SND_COMMAND 0x177

#define ATA_SND_ALT_STATUS 0x376
#define ATA_SND_DEV_CONTROL 0x376
#define ATA_SND_DRIVE_ADDRESS 0x377


// Bitmasks for ATA registers.
#define ATA_ERROR_NO_ADDRESS_MARK (1 << 0)
#define ATA_ERROR_NO_TRACK_ZERO (1 << 1)
#define ATA_ERROR_ABORTED (1 << 2)
#define ATA_ERROR_MEDIA_CHANGE_REQ (1 << 3)
#define ATA_ERROR_NO_ID (1 << 4)
#define ATA_ERROR_MEDIA_CHANGED (1 << 5)
#define ATA_ERROR_UNCORRECTABLE_DATA (1 << 6)
#define ATA_ERROR_BAD_BLOCK (1 << 7)

#define ATA_DRIVE_OR_HEAD_DRV0 (0 << 4)
#define ATA_DRIVE_OR_HEAD_DRV1 (1 << 4)
#define ATA_DRIVE_OR_HEAD_CHS (1 << 6)

#define ATA_STATUS_ERROR (1 << 0)
#define ATA_STATUS_INDEX (1 << 1)
#define ATA_STATUS_CORRECTED_DATA (1 << 2)
#define ATA_STATUS_DRQ (1 << 3)
#define ATA_STATUS_OVERLAPPED_REQ (1 << 4)
#define ATA_STATUS_DRIVE_FAULT (1 << 5)
#define ATA_STATUS_READY (1 << 6)
#define ATA_STATUS_BUSY (1 << 7)

#define ATA_DEV_CONTROL_INTERRUPT_DISABLE (1 << 1)
#define ATA_DEV_CONTROL_SOFTWARE_RESET (1 << 2)
#define ATA_DEV_CONTROL_HIGH_ORDER_BYTE (1 << 7)

#define ATA_DRIVE_ADDRESS_SELECT0 (1 << 0)
#define ATA_DRIVE_ADDRESS_SELECT1 (1 << 1)
#define ATA_DRIVE_ADDRESS_HEAD_SELECT_BASE (1 << 2)
#define ATA_DRIVE_ADDRESS_WRITE_GATE (1 << 6)


// ATA Commands.
#define ATA_COMMAND_IDENTIFY 0xEC

typedef enum {
    ATA_BUS_PRIM,
    ATA_BUS_SND,
} ATA_Bus;

#define ata_get_bus_port(bus, port_name) \
    ({ \
        uint16_t port = 0; \
        switch (bus) { \
            case ATA_BUS_PRIM: { \
                port = ATA_PRIM_##port_name; \
            } break; \
            case ATA_BUS_SND: { \
                port = ATA_SND_##port_name; \
            } break; \
            default: { \
                ASSERT(false, "Invalid bus."); \
            } break; \
        } \
        port; \
    })


void ata_drive_select(ATA_Bus bus, uint8_t drive) {
    ASSERT(drive == 0 || drive == 1, "Invalid drive number.");
    uint8_t alt_status = ATA_STATUS_BUSY | ATA_STATUS_DRQ;

    // Make sure the busy and DRQ bits are clear for this drive before switching.
    while ((alt_status & (ATA_STATUS_BUSY | ATA_STATUS_DRQ)) != 0) {
        uint16_t alt_status_port = ata_get_bus_port(bus, ALT_STATUS);
        alt_status = in_8(alt_status_port);
    }

    uint16_t drive_select_port = ata_get_bus_port(bus, DRIVE_OR_HEAD);
    out_8(drive_select_port, 0xA | (drive << 4));
}

bool ata_init() {
    // Check for floating bus.
    uint8_t primary_status = in_8(ATA_PRIM_STATUS);
    uint8_t secondary_status = in_8(ATA_SND_STATUS);

    fmt_print("primary_status: %hhx\n", primary_status);
    fmt_print("secondary_status: %hhx\n", secondary_status);

    // @TODO: Actually do stuff based on the floating bus results.
    ASSERT(primary_status != 0xFF, "Illegal value on primary bus.");
    ASSERT(secondary_status != 0xFF, "Illegal value on secondary bus.");

    // @Note: Exploration only. I just want to see how IDENTIFY works.
    ata_drive_select(ATA_BUS_PRIM, 0);
    uint16_t sector_count_port = ata_get_bus_port(ATA_BUS_PRIM, SECTOR_COUNT);
    out_8(sector_count_port, 0);
    uint16_t lba_low_port = ata_get_bus_port(ATA_BUS_PRIM, LBALOW_OR_SECTORNUM);
    out_8(lba_low_port, 0);
    uint16_t lba_mid_port = ata_get_bus_port(ATA_BUS_PRIM, LBAMID_OR_CYLINDERLOW);
    out_8(lba_mid_port, 0);
    uint16_t lba_high_port = ata_get_bus_port(ATA_BUS_PRIM, LBAHIGH_OR_CYLINDERHIGH);
    out_8(lba_high_port, 0);
    uint16_t command_port = ata_get_bus_port(ATA_BUS_PRIM, COMMAND);
    out_8(command_port, ATA_COMMAND_IDENTIFY);

    uint16_t alt_status_port = ata_get_bus_port(ATA_BUS_PRIM, ALT_STATUS);
    uint8_t alt_status = 0;
    while ((alt_status & (ATA_STATUS_DRQ | ATA_STATUS_ERROR)) == 0) {
        alt_status = in_8(alt_status_port);
    }

    ASSERT((alt_status & ATA_STATUS_ERROR) == 0, "Temporary: Error should not be set.");
    
    uint16_t identify_data[256] = {0};
    uint16_t data_port = ata_get_bus_port(ATA_BUS_PRIM, DATA);
    for (size_t i = 0; i < ARRAY_LEN(identify_data); i++) {
        identify_data[i] = in_16(data_port);
    }

    return true;
}
