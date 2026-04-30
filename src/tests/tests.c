// --------------------------------------------------
// - Page Frame Allocator
// --------------------------------------------------

bool test_page_frame_allocator_dummy() {
    return true;
}

Test_Suite suites[] = {
    TEST_SUITE(
        page_frame_allocator,
        .cleanup = pfa_uncommit_all,
        .tests = {
            TEST(test_page_frame_allocator_dummy),
        }
    ),
};
