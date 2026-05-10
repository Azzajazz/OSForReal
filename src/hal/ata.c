// Ports.
#define ATA_DATA(bus) ide_channels[bus].io_base + 0
#define ATA_ERROR(bus) ide_channels[bus].io_base + 1
#define ATA_FEATURES(bus) ide_channels[bus].io_base + 1
#define ATA_SECTOR_COUNT(bus) ide_channels[bus].io_base + 2
#define ATA_LBALOW_OR_SECTORNUM(bus) ide_channels[bus].io_base + 3
#define ATA_LBAMID_OR_CYLINDERLOW(bus) ide_channels[bus].io_base + 4
#define ATA_LBAHIGH_OR_CYLINDERHIGH(bus) ide_channels[bus].io_base + 5
#define ATA_DRIVE_OR_HEAD(bus) ide_channels[bus].io_base + 6
#define ATA_STATUS(bus) ide_channels[bus].io_base + 7
#define ATA_COMMAND(bus) ide_channels[bus].io_base + 7

#define ATA_ALT_STATUS(bus) ide_channels[bus].control_base + 0
#define ATA_DEV_CONTROL(bus) ide_channels[bus].control_base + 0
#define ATA_DRIVE_ADDRESS(bus) ide_channels[bus].control_base + 1

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
#define ATA_DRIVE_OR_HEAD_LBA (1 << 6)

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
#define ATA_COMMAND_READ_SECTORS 0x20
#define ATA_COMMAND_WRITE_SECTORS 0x30

IDE_Channel_Info ide_channels[2] = {0};
IDE_Drive_Info ide_drives[2][2] = {0};

void ide_select_drive(IDE_Bus_ID bus, uint8_t drive) {
    ASSERT(drive == 0 || drive == 1, "Invalid drive number.");
    out_8(ATA_DRIVE_OR_HEAD(bus), 0xA | (drive << 4));
    // We need to wait 400ns for the drive select to go through. Apparently, each
    // `in` instruction takes ~100ns, so doing it 4 times (and ignoring the results)
    // is necessary.
    for (int i = 0; i < 4; i++) {
        in_8(ATA_STATUS(bus));
    }
}

bool ide_init(bool primary_pci_native, bool secondary_pci_native, uint16_t bar0, uint16_t bar1, uint16_t bar2, uint16_t bar3, uint16_t bar4) {
    UNUSED(bar4);
    if (primary_pci_native) {
        ide_channels[IDE_BUS_PRIM].io_base = bar0;
        ide_channels[IDE_BUS_PRIM].control_base = bar1;
    }
    else {
        ide_channels[IDE_BUS_PRIM].io_base = 0x1F0;
        ide_channels[IDE_BUS_PRIM].control_base = 0x3F6;
    }
    if (secondary_pci_native) {
        ide_channels[IDE_BUS_PRIM].io_base = bar2;
        ide_channels[IDE_BUS_PRIM].control_base = bar3;
    }
    else {
        ide_channels[IDE_BUS_SND].io_base = 0x170;
        ide_channels[IDE_BUS_SND].control_base = 0x376;
    }

    // Check for floating bus.
    uint8_t primary_status = in_8(ATA_STATUS(IDE_BUS_PRIM));
    uint8_t secondary_status = in_8(ATA_STATUS(IDE_BUS_SND));
    if (primary_status == 0xFF) {
        // Primary bus has no drives.
        ide_drives[IDE_BUS_PRIM][0].missing = true;
        ide_drives[IDE_BUS_PRIM][1].missing = true;
    }
    if (secondary_status == 0xFF) {
        // Secondary bus has no drives.
        ide_drives[IDE_BUS_SND][0].missing = true;
        ide_drives[IDE_BUS_SND][1].missing = true;
    }

    uint16_t identify_data[256] = {0};

    // Identify the drives.
    for (size_t bus = 0; bus < 2; bus++) {
        for (size_t drive = 0; drive < 2; drive++) {
            if (ide_drives[bus][drive].missing) {
                continue;
            }

            ide_select_drive(bus, drive);

            // Poll until BSY and DRQ are clear.
            uint8_t status = in_8(ATA_STATUS(bus));
            while (status & (ATA_STATUS_BUSY | ATA_STATUS_DRQ)) {
                status = in_8(ATA_STATUS(bus));
            }

            out_8(ATA_SECTOR_COUNT(bus), 0);
            out_8(ATA_LBALOW_OR_SECTORNUM(bus), 0);
            out_8(ATA_LBAMID_OR_CYLINDERLOW(bus), 0);
            out_8(ATA_LBAHIGH_OR_CYLINDERHIGH(bus), 0);
            out_8(ATA_COMMAND(bus), ATA_COMMAND_IDENTIFY);

            if (status == 0) {
                ide_drives[bus][drive].missing = true;
                continue;
            }

            // Poll until BSY clears. The status register is meaningless until then.
            while (status & ATA_STATUS_BUSY) {
                status = in_8(ATA_STATUS(bus));
            }

            // @TODO: There's a section on the ATA PIO mode osdev.org page that talks about
            // identifying non-standard ATAPI devices here. Implement that.

            // Poll until either ERR sets (i.e. there was an error) or DRQ sets
            // (i.e, the drive is ready for data read).
            while ((status & (ATA_STATUS_ERROR | ATA_STATUS_DRQ)) == 0) {
                status = in_8(ATA_STATUS(bus));
            }
            if (status & ATA_STATUS_ERROR) {
                // @TODO: Better drive type detection.
                ide_drives[bus][drive].type = DRIVE_TYPE_UNKNOWN;
                continue;
            }
            else {
                ide_drives[bus][drive].type = DRIVE_TYPE_ATA;
            }

            // Read the identify data.
            for (size_t i = 0; i < ARRAY_LEN(identify_data); i++) {
                identify_data[i] = in_16(ATA_DATA(bus));
            }

            uint8_t supported_udma_modes = identify_data[88] & 0xFF;
            uint8_t active_udma_mode = identify_data[88] >> 8;
            if (__builtin_clz(supported_udma_modes) < __builtin_clz(active_udma_mode)) {
                fmt_print("WARNING: The active udma mode is not the maximum supported.\n");
                fmt_print("  bus: %hhx, drive: %hhx\n", bus, drive);
                fmt_print("  active bitmask: %hhx, supported bitmask: %hhx\n", active_udma_mode, supported_udma_modes);
            }
            ide_drives[bus][drive].udma_mode = __builtin_ffs(active_udma_mode);
            // @TODO: Check the 80 line on the master drive. That may limit the udma version we can use.

            ide_drives[bus][drive].lba28_addressable_sector_count =
                identify_data[60] | ((uint32_t)identify_data[61] << 16);

            bool supports_lba_48 = (identify_data[83] & (1 << 10)) != 0;
            if (supports_lba_48) {
                ide_drives[bus][drive].lba48_addressable_sector_count =
                    identify_data[100] |
                    ((uint64_t)identify_data[101] << 16) |
                    ((uint64_t)identify_data[102] << 32) |
                    ((uint64_t)identify_data[103] << 48);
            }
        }
    }

    return true;
}

void ata_prepare_transfer(IDE_Bus_ID bus, uint8_t drive, uint32_t lba28) {
    ASSERT(drive == 0 || drive == 1, "Invalid drive number.");
    ASSERT(lba28 <= 0xFFFFFFF, "lba28 can be at most 28 bits.");

    uint8_t drive_or_head_value = 
        0xA0 | (drive << 4) | ATA_DRIVE_OR_HEAD_LBA | ((lba28 >> 24) & 0x0F);
    out_8(ATA_DRIVE_OR_HEAD(bus), drive_or_head_value);
    out_8(ATA_SECTOR_COUNT(bus), 1); // @TODO: Support multiple sector reads.
    out_8(ATA_LBALOW_OR_SECTORNUM(bus), lba28 & 0xFF);
    out_8(ATA_LBAMID_OR_CYLINDERLOW(bus), (lba28 >> 8) & 0xFF);
    out_8(ATA_LBAHIGH_OR_CYLINDERHIGH(bus), (lba28 >> 16) & 0xFF);
}

// @NOTE: The caller must ensure the buffer can hold one 512-byte sector.
void ata_read_sector(IDE_Bus_ID bus, uint8_t drive, uint32_t lba28, void *buffer) {
    ide_select_drive(bus, drive);
    ata_prepare_transfer(bus, drive, lba28);
    out_8(ATA_COMMAND(bus), ATA_COMMAND_READ_SECTORS);
    
    // Poll until ERR and DF are clear. They may be set from the previous
    // command for a little bit.
    uint8_t status = in_8(ATA_STATUS(bus));

    // Poll until BSY clears. The status register is meaningless until then.
    while (status & ATA_STATUS_BUSY) {
        status = in_8(ATA_STATUS(bus));
    }

    // Poll until DRQ is set (i.e, there is data to be read) or ERR/DF is set
    // (i.e, something went wrong).
    while ((status & (ATA_STATUS_DRQ | ATA_STATUS_ERROR | ATA_STATUS_DRIVE_FAULT)) == 0) {
        status = in_8(ATA_STATUS(bus));
    }
    ASSERT((status & (ATA_STATUS_ERROR | ATA_STATUS_DRIVE_FAULT)) == 0, "Error should not occur.");

    // @TODO: Alignment applies here; we should make sure that buffer is uint16_t aligned.
    // This requires adding an alignment parameter to memory_allocate.
    uint16_t *buffer_16 = (uint16_t *)buffer;
    for (size_t i = 0; i < 256; i++) {
        buffer_16[i] = in_16(ATA_DATA(bus));
    }
}

// @NOTE: The caller must ensure the buffer contains 512 bytes of data.
void ata_write_sector(IDE_Bus_ID bus, uint8_t drive, uint32_t lba28, void *buffer) {
    ide_select_drive(bus, drive);
    ata_prepare_transfer(bus, drive, lba28);
    out_8(ATA_COMMAND(bus), ATA_COMMAND_WRITE_SECTORS);
    
    // Poll until ERR and DF are clear. They may be set from the previous
    // command for a little bit.
    uint8_t status = in_8(ATA_STATUS(bus));

    // Poll until BSY clears. The status register is meaningless until then.
    while (status & ATA_STATUS_BUSY) {
        status = in_8(ATA_STATUS(bus));
    }

    // Poll until DRQ is set (i.e, there is data to be read) or ERR/DF is set
    // (i.e, something went wrong).
    while ((status & (ATA_STATUS_DRQ | ATA_STATUS_ERROR | ATA_STATUS_DRIVE_FAULT)) == 0) {
        status = in_8(ATA_STATUS(bus));
    }
    ASSERT((status & (ATA_STATUS_ERROR | ATA_STATUS_DRIVE_FAULT)) == 0, "Error should not occur.");

    // @TODO: Alignment applies here; we should make sure that buffer is uint16_t aligned.
    // This requires adding an alignment parameter to memory_allocate.
    uint16_t *buffer_16 = (uint16_t *)buffer;
    for (size_t i = 0; i < 256; i++) {
        out_16(ATA_DATA(bus), buffer_16[i]);
    }
}
