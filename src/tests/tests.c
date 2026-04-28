extern int __boot_start;
extern int __kernel_phys_end;

// NOTE: This test has several hardcoded values, which may be invalid depending on the
// QEMU configuration you are using. Tread with caution.
bool kernel_test_boot_info_is_correct(Bootstrap_Info info) {
    size_t page_directory = (size_t)info.page_directory;
    size_t page_directory_size = 4096;

    size_t page_tables = (size_t)info.page_tables;
    size_t page_tables_size = 1024 * 1024 * 4;

    size_t page_bitmap = (size_t)info.page_bitmap;
    size_t page_bitmap_size = info.page_bitmap_size;

    size_t code_start = (size_t)&__boot_start;
    size_t code_size = (size_t)&__kernel_phys_end - code_start;

    // These constants can come from the Multiboot_Info, but it's not so simple to preserve
    // those values until here. These values shouldn't change much, so hardcoding them
    // is fine for now.
    size_t unusable_ram_blocks[][2] = {
        {0x9F000, 397312u},
        {0x7FE0000, 4160880640u},
    };
    
    for (uint64_t addr = 0; addr < 0x100000000; addr += 4096) {
        int mask_offset = (addr / 4096) / 32;
        int bit_offset = (addr / 4096) % 32;
        uint32_t *entry = (uint32_t *)info.page_bitmap + mask_offset;

        // NOTE: All the sizes above are multiples of the page size, so that makes the
        // logic in here a lot easier.
        bool page_contains_page_directory = (addr >= page_directory && addr < page_directory + page_directory_size);
        bool page_contains_page_tables = (addr >= page_tables && addr < page_tables + page_tables_size);
        bool page_contains_page_bitmap = (addr >= page_bitmap && addr < page_bitmap + page_bitmap_size);
        bool page_contains_code = (addr >= code_start && addr < code_start + code_size);

        bool unusable = false;
        for (size_t i = 0; i < ARRAY_LEN(unusable_ram_blocks); i++) {
            size_t base_addr = unusable_ram_blocks[i][0];
            size_t length = unusable_ram_blocks[i][1];
            if (addr >= base_addr && addr - base_addr < length) {
                unusable = true;
                break;
            }
        }

        if (unusable || page_contains_page_directory || page_contains_page_tables || page_contains_page_bitmap || page_contains_code) {
            TEST_ASSERT((*entry & (1 << bit_offset)) != 0);
        }
        else {
            TEST_ASSERT((*entry & (1 << bit_offset)) == 0);
        }
    }

    return true;
}

Test tests[] = {
    //TEST(kernel_test_boot_info_is_correct),
};
