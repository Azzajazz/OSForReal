#include "../kernel.c"

typedef bool (*Test_Fn)();

typedef struct {
    char *name;
    Test_Fn function;
} Test;

#define TEST(test_name) \
    (Test){.name = #test_name, .function = test_name}

bool dummy_test() {
    return true;
}

Test tests[] = {
    TEST(dummy_test),
};

void kernel_test(Multiboot_Info *boot_info) {
    UNUSED(boot_info);
    kernel_init();

    int passed = 0;
    int failed = 0;

    for (size_t i = 0; i < ARRAY_LEN(tests); i++) {
        Test test = tests[i];
        fmt_print("Running test %s... ", test.name);

        bool result = test.function();
        if (result) {
            fmt_print("OK!\n");
            passed += 1;
        }
        else {
            fmt_print("FAIL!\n");
            failed += 1;
        }

        fmt_print("--------------------\n");
        fmt_print("PASSED: %d, FAILED: %d\n", passed, failed);
    }

    cpu_halt();
}
