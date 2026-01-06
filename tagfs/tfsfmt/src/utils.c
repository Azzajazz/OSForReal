#define PACKED __attribute__((packed))

typedef struct PACKED {
    uint32_t jump;
    uint32_t version;
    uint32_t sector_count;
    uint32_t sector_size;
    uint32_t file_meta_sector_count;
    uint32_t tag_meta_sector_count;
    uint32_t tag_file_sector_count;
    uint32_t fat_sector_count;
    uint16_t free_file_id;
    uint16_t free_tag_id;
} FS_Metadata;

typedef struct PACKED {
    uint16_t id;
    uint32_t first_data_sector;
    uint32_t size;
    char name[22];
} File_Metadata;

typedef struct PACKED {
    uint16_t id;
    uint32_t first_data_sector;
    char name[26];
} Tag_Metadata;

typedef struct PACKED {
    uint16_t tag_id;
    uint16_t file_id;
} Tag_File_Entry;

void read_fs_metadata(FS_Metadata *fs_meta, int fd) {
    // @TODO: Read until we completely fill up the FS_Metadata
    lseek(fd, 0, SEEK_SET);
    read(fd, fs_meta, sizeof(*fs_meta));
}

File_Metadata *get_file_metadata(uint8_t *base, FS_Metadata *fs_meta) {
    return (File_Metadata*)(base + fs_meta->sector_size);
}

Tag_Metadata *get_tag_metadata(uint8_t *base, FS_Metadata *fs_meta) {
    return (Tag_Metadata*)(
        base + (
            1 +
            fs_meta->file_meta_sector_count
        ) * fs_meta->sector_size
    );
}

Tag_File_Entry *get_tag_file_array(uint8_t *base, FS_Metadata *fs_meta) {
    return (Tag_File_Entry*)(
        base + (
            1 +
            fs_meta->file_meta_sector_count +
            fs_meta->tag_meta_sector_count
        ) * fs_meta->sector_size
    );
}

uint16_t *get_fat(uint8_t *base, FS_Metadata *fs_meta) {
    return (uint16_t*)(
        base + (
            1 +
            fs_meta->file_meta_sector_count +
            fs_meta->tag_meta_sector_count +
            fs_meta->tag_file_sector_count
        ) * fs_meta->sector_size
    );
}

uint8_t *get_data(uint8_t *base, FS_Metadata *fs_meta) {
    return (
        base + (
            1 +
            fs_meta->file_meta_sector_count +
            fs_meta->tag_meta_sector_count +
            fs_meta->tag_file_sector_count +
            fs_meta->fat_sector_count
        ) * fs_meta->sector_size
    );
}
