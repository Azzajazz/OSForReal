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
} TagFS_FS_Metadata;

typedef struct PACKED {
    uint32_t first_data_sector;
    uint32_t size;
    char name[24];
} TagFS_File_Metadata;

typedef struct PACKED {
    char name[32];
} TagFS_Tag_Metadata;

typedef struct PACKED {
    uint16_t tag_id;
    uint16_t file_id;
} TagFS_Tag_File_Entry;

bool tfs_init();
bool tfs_get_file_attributes(String path, TagFS_File_Metadata *file_meta);
bool tfs_read(String path, uint8_t *buffer, size_t read_size, size_t offset);
bool tfs_write(String path, uint8_t *buffer, size_t size, size_t offset);
