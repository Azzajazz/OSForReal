// NOTE: This test has several hardcoded values, which may be invalid depending on the
// QEMU configuration you are using. Tread with caution.
bool kernel_test_boot_info_is_correct(Bootstrap_Info info) {
#define PAGE_SIZE 4096
    // The bitmap should cover all pages.
    size_t page_count = 0x100000;
    TEST_ASSERT(info.page_bitmap_size == page_count / 8);

    // Check that the boot and kernel pages are all reserved.
    size_t reserved_start_page = 0x100000 / PAGE_SIZE;
    size_t reserved_end_page = 0x10A000 / PAGE_SIZE;
    size_t reserved_page_count = reserved_end_page - reserved_start_page;
    fmt_print("%x: %u\n", reserved_page_count, reserved_page_count);

    uint32_t *page_bitmap = info.page_bitmap;
    uint32_t entry = page_bitmap[reserved_start_page / 32];
    fmt_print("%x\n", entry);

    return true;
}

Test tests[] = {
    TEST(kernel_test_boot_info_is_correct),
};
