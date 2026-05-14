TagFS_FS_Metadata tfs_fs_meta = {0};
uint8_t *tfs_sector_buffer = 0;

bool tfs_init(/* @TODO: Pass in the IDE device */) {
    tfs_sector_buffer = memory_allocate(512);
    // Read the FS metadata sector.
    ata_read_sector(IDE_BUS_PRIM, 0, 0, tfs_sector_buffer);
    memory_copy(&tfs_fs_meta, tfs_sector_buffer, sizeof(TagFS_FS_Metadata));
    return true;
}

static inline uint32_t tfs_get_file_metadata_lba() {
    return 1;
}

static inline uint32_t tfs_get_tag_metadata_lba() {
    return tfs_get_file_metadata_lba() +
        tfs_fs_meta.file_meta_sector_count;
}

static inline uint32_t tfs_get_tag_file_array_lba() {
    return tfs_get_tag_metadata_lba() +
        tfs_fs_meta.tag_meta_sector_count;
}

static inline uint32_t tfs_get_fat_lba() {
    return tfs_get_tag_file_array_lba() +
        tfs_fs_meta.tag_file_sector_count;
}

static inline uint32_t tfs_get_data_lba() {
    return tfs_get_fat_lba() +
        tfs_fs_meta.fat_sector_count;
}

static inline size_t tfs_count_file_meta_entries() {
    return tfs_fs_meta.file_meta_sector_count * tfs_fs_meta.sector_size / sizeof(TagFS_File_Metadata);
}

typedef struct {
    bool found;
    uint32_t lba;
    size_t offset;
} File_Metadata_Result;

File_Metadata_Result tfs_get_file_metadata_from_name(String path, TagFS_File_Metadata *file_meta) {
    File_Metadata_Result result = {0};
    uint32_t file_meta_lba = tfs_get_file_metadata_lba();
    size_t file_meta_entries_per_sector = tfs_fs_meta.sector_size / sizeof(TagFS_File_Metadata);
    size_t file_count = 0;

    for (
        size_t sector_offset = 0;
        sector_offset < tfs_fs_meta.file_meta_sector_count;
        sector_offset++
    ) {
        ata_read_sector(IDE_BUS_PRIM, 0, file_meta_lba, tfs_sector_buffer);
        TagFS_File_Metadata *buf_file_meta = (TagFS_File_Metadata *)tfs_sector_buffer;

        for (size_t i = 0; i < file_meta_entries_per_sector; i++) {
            if (str_equals_cstr(path, buf_file_meta[i].name)) {
                memory_copy(file_meta, &buf_file_meta[i], sizeof(*file_meta));
                result.found = true;
                result.lba = file_meta_lba;
                result.offset = i * sizeof(TagFS_File_Metadata);
                break;
            }
            file_count++;
            if (file_count > tfs_count_file_meta_entries()) {
                break;
            }
        }

        file_meta_lba++;
    }

    return result;
}

int tfs_find_free_fat(uint16_t *fats) {
    int fat_count = tfs_fs_meta.fat_sector_count * tfs_fs_meta.sector_size / sizeof(uint16_t);

    for (int i = 0; i < fat_count; i++) {
        if (fats[i] == 0) {
            return i;
        }
    }

    return -1;
}

bool tfs_get_file_attributes(String path, TagFS_File_Metadata *file_meta) {
    File_Metadata_Result meta_result = tfs_get_file_metadata_from_name(path, file_meta);
    return meta_result.found;
}

// @TODO: Return some sort of status code, or maybe how many bytes were read.
bool tfs_read(String path, uint8_t *buffer, size_t read_size, size_t offset) {
    ASSERT(tfs_fs_meta.fat_sector_count == 2, "Unsupported sector count.");
    bool result = true;

    size_t fats_size = tfs_fs_meta.sector_size * tfs_fs_meta.fat_sector_count;
    // @TODO: Allocate this buffer only once. In future, we may need to
    // load the FAT sectors lazily since the FAT section may be too large to store
    // in memory. But then again, maybe not. Look into this.
    uint16_t *fats = memory_allocate(fats_size);

    uint32_t fat_lba = tfs_get_fat_lba();
    ata_read_sector(IDE_BUS_PRIM, 0, fat_lba, fats);
    ata_read_sector(IDE_BUS_PRIM, 0, fat_lba + 1, fats + tfs_fs_meta.sector_size);

    TagFS_File_Metadata *file_meta = memory_allocate(sizeof(*file_meta));
    File_Metadata_Result meta_result = tfs_get_file_metadata_from_name(path, file_meta);
    if (!meta_result.found) {
        result = false;
        goto done;
    }

    size_t sector_skip_count = offset / tfs_fs_meta.sector_size;
    size_t byte_skip_count = offset % tfs_fs_meta.sector_size;

    // Skip the number of sectors specified by the offset.
    size_t fat_index = file_meta->first_data_sector;
    for (size_t i = 0; i < sector_skip_count; i++) {
        if (fats[fat_index] == 0xffff) {
            result = false;
            goto done;
        }
        fat_index = fats[fat_index];
    }

    uint32_t data_lba = tfs_get_data_lba();
    uint32_t sector_lba = data_lba + fat_index;
    ata_read_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);
    if (read_size <= tfs_fs_meta.sector_size - byte_skip_count) {
        // We're just reading from one sector.
        memory_copy(buffer, tfs_sector_buffer + byte_skip_count, read_size);
        goto done;
    }

    else {
        size_t buf_index = 0;

        // We're reading multiple sectors. Read as much of the current sector as we can.
        size_t first_sector_bytes = tfs_fs_meta.sector_size - byte_skip_count;
        memory_copy(buffer, tfs_sector_buffer + byte_skip_count, first_sector_bytes);
        buf_index += first_sector_bytes;

        // Now read the rest.
        while (buf_index < read_size) {
            if (buf_index >= file_meta->size) {
                goto done;
            }
            fat_index = fats[fat_index];

            size_t bytes_to_read = tfs_fs_meta.sector_size;
            if (read_size <= buf_index + tfs_fs_meta.sector_size) {
                bytes_to_read = read_size - buf_index;
            }

            sector_lba = data_lba + fat_index;
            ata_read_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);
            memory_copy(buffer + buf_index, tfs_sector_buffer, bytes_to_read);
            buf_index += bytes_to_read;
        }
        goto done;
    }

done:
    memory_free(file_meta);
    memory_free(fats);
    return result;
}

bool tfs_write(String path, uint8_t *buffer, size_t size, size_t offset) {
    ASSERT(tfs_fs_meta.fat_sector_count == 2, "Unsupported sector count.");
    bool result = true;

    size_t fats_size = tfs_fs_meta.sector_size * tfs_fs_meta.fat_sector_count;
    // @TODO: Allocate this buffer only once. In future, we may need to
    // load the FAT sectors lazily since the FAT section may be too large to store
    // in memory. But then again, maybe not. Look into this.
    uint16_t *fats = memory_allocate(fats_size);

    uint32_t fat_lba = tfs_get_fat_lba();
    ata_read_sector(IDE_BUS_PRIM, 0, fat_lba, fats);
    ata_read_sector(IDE_BUS_PRIM, 0, fat_lba + 1, fats + tfs_fs_meta.sector_size);

    TagFS_File_Metadata *file_meta = memory_allocate(sizeof(*file_meta));
    File_Metadata_Result meta_result = tfs_get_file_metadata_from_name(path, file_meta);
    if (!meta_result.found) {
        result = false;
        goto done;
    }

    int fat_index;
    if (file_meta->first_data_sector == 0xffff) {
        fat_index = tfs_find_free_fat(fats);
        if (fat_index == -1) {
            result = false;
            goto done;
        }
        file_meta->first_data_sector = fat_index;
    }
    else {
        fat_index = file_meta->first_data_sector;
    }

    size_t byte_skip_count = 0;
    if (offset < file_meta->size) {
        // Skip the required amount of sectors.
        size_t sector_skip_count = offset / tfs_fs_meta.sector_size;
        byte_skip_count = offset % tfs_fs_meta.sector_size;

        // Skip the number of sectors specified by the offset.
        for (size_t i = 0; i < sector_skip_count; i++) {
            if (fats[fat_index] == 0xffff) {
                result = false;
                goto done;
            }
            fat_index = fats[fat_index];
        }
    }
    else {
        size_t sector_skip_count = file_meta->size / tfs_fs_meta.sector_size;
        byte_skip_count = file_meta->size % tfs_fs_meta.sector_size;

        for (size_t i = 0; i < sector_skip_count; i++) {
            if (fats[fat_index] == 0xffff) {
                result = false;
                goto done;
            }
            fat_index = fats[fat_index];
        }
    }

    size_t data_lba = tfs_get_data_lba();
    if (size <= tfs_fs_meta.sector_size - byte_skip_count) {
        size_t sector_lba = data_lba + fat_index;
        ata_read_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);
        memory_copy(&tfs_sector_buffer[byte_skip_count], buffer, size);
        ata_write_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);

        if (fats[fat_index] == 0) {
            fats[fat_index] = 0xffff;
        }
        if (offset + size > file_meta->size) {
            file_meta->size = offset + size;
        }
    }
    else {
        size_t buf_index = 0;

        // We're writing multiple sectors. Write as much of the current sector as we can.
        size_t first_sector_bytes = tfs_fs_meta.sector_size - byte_skip_count;
        size_t sector_lba = data_lba + fat_index;
        ata_read_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);
        memory_copy(&tfs_sector_buffer[byte_skip_count], buffer, first_sector_bytes);
        ata_write_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);
        buf_index += first_sector_bytes;

        // Now write the rest.
        while (buf_index < size) {
            // Find the first free FAT.
            int fat_index = tfs_find_free_fat(fats);
            if (fat_index == -1) {
                break;
            }

            size_t bytes_to_write = tfs_fs_meta.sector_size;
            if (size <= buf_index + tfs_fs_meta.sector_size) {
                bytes_to_write = size - buf_index;
            }

            size_t sector_lba = data_lba + fat_index;
            ata_read_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);
            memory_copy(tfs_sector_buffer, &buffer[buf_index], bytes_to_write);
            ata_write_sector(IDE_BUS_PRIM, 0, sector_lba, tfs_sector_buffer);
            buf_index += bytes_to_write;
        }

        if (offset + buf_index > file_meta->size) {
            file_meta->size = offset + buf_index;
        }
    }

    // Write the file meta entry back to the disk.
    ata_read_sector(IDE_BUS_PRIM, 0, meta_result.lba, tfs_sector_buffer);
    memory_copy(&tfs_sector_buffer[meta_result.offset], file_meta, sizeof(*file_meta));
    ata_write_sector(IDE_BUS_PRIM, 0, meta_result.lba, tfs_sector_buffer);

done:
    memory_free(file_meta);
    memory_free(fats);
    return result;
}
