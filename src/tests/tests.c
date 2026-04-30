// --------------------------------------------------
// - Page Frame Allocator
// --------------------------------------------------

bool test_page_frame_allocator_virt_to_tables_index() {
    void *virt = (void *)0xC0800000;
    TEST_ASSERT(mapped_virt_to_pfa_tables_index(virt) == 0);
    virt = (void *)0xC0801000;
    TEST_ASSERT(mapped_virt_to_pfa_tables_index(virt) == 0);
    virt = (void *)0xC0C00000;
    TEST_ASSERT(mapped_virt_to_pfa_tables_index(virt) == 1);
    virt = (void *)0xC0C12000;
    TEST_ASSERT(mapped_virt_to_pfa_tables_index(virt) == 1);
    virt = (void *)0xFFFFFFFF;
    TEST_ASSERT(mapped_virt_to_pfa_tables_index(virt) == 253);
    return true;
}

bool test_page_frame_allocator_tables_index_to_virt() {
    size_t index = 0;
    TEST_ASSERT(pfa_tables_index_to_mapped_virt(index) == (void *)0xC0800000);
    index = 1;
    TEST_ASSERT(pfa_tables_index_to_mapped_virt(index) == (void *)0xC0C00000);
    index = 2;
    TEST_ASSERT(pfa_tables_index_to_mapped_virt(index) == (void *)0xC1000000);
    index = 253;
    TEST_ASSERT(pfa_tables_index_to_mapped_virt(index) == (void *)0xFFC00000);
    return true;
}

bool test_page_frame_allocator_commit_one_page() {
    void *virt = (void *)0xC0800000;
    pfa_commit_page(virt);

    size_t page_tables_index = mapped_virt_to_pfa_tables_index(virt);
    size_t page_table_phys = pfa_tables_index_to_table_phys(page_tables_index);
    size_t page_table_entry = mapped_virt_to_page_table_entry(virt);
    size_t page_directory_entry = mapped_virt_to_page_directory_entry(virt);
    TEST_ASSERT(page_directory[page_directory_entry] == ((size_t)page_table_phys | 0x27));
    TEST_ASSERT(pfa_page_tables[page_tables_index][page_table_entry] == 0xa00027);

    TEST_ASSERT(pfa_page_bitmap[0] == 0x00000001);

    return true;
}

bool test_page_frame_allocator_resets_fully_after_uncommit_all() {
    // Two in the same page table.
    void *virt1 = (void *)0xC0800000;
    pfa_commit_page(virt1);
    void *virt2 = (void *)0xC0801000;
    pfa_commit_page(virt2);

    // One in a different page table.
    void *virt3 = (void *)0xC0C00000;
    pfa_commit_page(virt3);

    pfa_uncommit_all();

    size_t page_directory_entry1 = mapped_virt_to_page_directory_entry(virt1);
    size_t page_directory_entry2 = mapped_virt_to_page_directory_entry(virt2);
    size_t page_directory_entry3 = mapped_virt_to_page_directory_entry(virt3);
    TEST_ASSERT(page_directory[page_directory_entry1] == 0);
    TEST_ASSERT(page_directory[page_directory_entry2] == 0);
    TEST_ASSERT(page_directory[page_directory_entry3] == 0);

    for (size_t i = 0; i < ARRAY_LEN(pfa_page_tables); i++) {
        for (size_t j = 0; j < ARRAY_LEN(pfa_page_tables[i]); j++) {
            TEST_ASSERT(pfa_page_tables[i][j] == 0);
        }
    }

    for (size_t i = 0; i < ARRAY_LEN(pfa_page_bitmap); i++) {
        TEST_ASSERT(pfa_page_bitmap[i] == 0);
    }

    return true;
}

bool test_page_frame_allocator_can_commit_multiple_pages() {
    // Two in the same page table.
    void *virt1 = (void *)0xC0800000;
    pfa_commit_page(virt1);
    void *virt2 = (void *)0xC0801000;
    pfa_commit_page(virt2);

    // One in a different page table.
    void *virt3 = (void *)0xC0C00000;
    pfa_commit_page(virt3);

    size_t low_page_tables_index = mapped_virt_to_pfa_tables_index(virt1);
    size_t low_page_table_phys = pfa_tables_index_to_table_phys(low_page_tables_index);
    size_t page_table_entry1 = mapped_virt_to_page_table_entry(virt1);
    size_t page_table_entry2 = mapped_virt_to_page_table_entry(virt2);
    size_t low_page_directory_entry = mapped_virt_to_page_directory_entry(virt1);
    TEST_ASSERT(page_directory[low_page_directory_entry] == ((size_t)low_page_table_phys | 0x27));
    TEST_ASSERT(pfa_page_tables[low_page_tables_index][page_table_entry1] == 0xa00027);
    TEST_ASSERT(pfa_page_tables[low_page_tables_index][page_table_entry2] == 0xa01027);

    size_t high_page_tables_index = mapped_virt_to_pfa_tables_index(virt3);
    size_t high_page_table_phys = pfa_tables_index_to_table_phys(high_page_tables_index);
    size_t page_table_entry3 = mapped_virt_to_page_table_entry(virt3);
    size_t high_page_directory_entry = mapped_virt_to_page_directory_entry(virt3);
    TEST_ASSERT(page_directory[high_page_directory_entry] == ((size_t)high_page_table_phys | 0x27));
    TEST_ASSERT(pfa_page_tables[high_page_tables_index][page_table_entry3] == 0xa02027);

    TEST_ASSERT(pfa_page_bitmap[0] == 0x00000007);

    return true;
}

bool test_page_frame_allocator_commit_more_than_32_pages_bitmap_is_correct() {
    for (size_t virt_s = 0xC0800000; virt_s < 0xC0800000 + 33 * PAGE_SIZE; virt_s += PAGE_SIZE) {
        pfa_commit_page((void *)virt_s);
    }

    TEST_ASSERT(pfa_page_bitmap[0] == 0xffffffff);
    TEST_ASSERT(pfa_page_bitmap[1] == 0x00000001);

    return true;
}

Test_Suite suites[] = {
    TEST_SUITE(
        page_frame_allocator,
        .cleanup = pfa_uncommit_all,
        .tests = {
            TEST(test_page_frame_allocator_virt_to_tables_index),
            TEST(test_page_frame_allocator_tables_index_to_virt),
            TEST(test_page_frame_allocator_commit_one_page),
            TEST(test_page_frame_allocator_resets_fully_after_uncommit_all),
            TEST(test_page_frame_allocator_can_commit_multiple_pages),
            TEST(test_page_frame_allocator_commit_more_than_32_pages_bitmap_is_correct),
        }
    ),
};
