// NOTE: This test has several hardcoded values, which may be invalid depending on the
// QEMU configuration you are using. Tread with caution.
bool kernel_test_boot_info_is_correct(Bootstrap_Info info) {
    // These constants can come from the Bootstrap_Info, but it seems self-referential
    // to test the values in the Bootstrap_Info using information from itself.
    // These constants are easy enough to verify ourselves, so it seems a good middle ground.
    size_t page_directory = 0;
    size_t page_directory_size = 4096;

    size_t page_tables = 0x10E000;
    size_t page_tables_size = 1024 * 1024 * 4;

    size_t page_bitmap = 0x1000;
    size_t page_bitmap_size = 131072;

    size_t code_start = 0x100000;
    size_t code_size = 57344;

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

        if (page_contains_page_directory || page_contains_page_tables || page_contains_page_bitmap || page_contains_code) {
            TEST_ASSERT((*entry & (1 << bit_offset)) != 0);
        }
        else {
            // @TODO: Entries marked as unavailable by multi-boot should be 1 as well...
            TEST_ASSERT((*entry & (1 << bit_offset)) == 0);
        }
    }

    return true;
}

Test tests[] = {
    TEST(kernel_test_boot_info_is_correct),
};
