#include "../kernel.c"

typedef bool (*Test_Fn)();

typedef struct {
    char *name;
    Test_Fn function;
} Test;

#ifndef TEST_SUITE_MAX_TESTS
#define TEST_SUITE_MAX_TESTS 32
#endif
typedef struct {
    // Called before running a test in this suite.
    void (*setup)(void);
    // Called after running a test in this suite.
    void (*cleanup)(void);
    char *name;

    // @TODO: This uses static memory, which wastes a lot of space and increases the size of the kernel.
    // I can imagine having tests use dynamic arrays by default, and be able to specify
    // that we want a suite to use static memory instead (needed, e.g, when testing the allocators themselves).
    Test tests[TEST_SUITE_MAX_TESTS];
} Test_Suite;

#define TEST(test_name) \
    (Test){.name = #test_name, .function = test_name}

#define TEST_SUITE(suite_name, ...) \
    (Test_Suite){.name = #suite_name, __VA_ARGS__}

bool test_assert_impl(char *file, int line, char *cond_str, bool cond) {
    if (!(cond)) { \
        fmt_print("\nASSERTION FAILED\n");
        fmt_print("%s:%d: %s is false.\n", file, line, cond_str);
        return false;
    }

    return true;
}

#define TEST_ASSERT(cond) \
    if (!test_assert_impl(__FILE__, __LINE__, #cond, cond)) return false

#include "tests.c"

void kernel_test(Multiboot_Info *boot_info) {
    kernel_init(boot_info);

    int passed = 0;
    int failed = 0;

    for (size_t i = 0; i < ARRAY_LEN(suites); i++) {
        Test_Suite suite = suites[i];

        fmt_print("\n---------- SUITE %s ----------\n", suite.name);

        for (size_t j = 0; j < ARRAY_LEN(suite.tests); j++) {
            Test test = suite.tests[j];
            if (test.function == 0) {
                break;
            }

            fmt_print("    Running test %s... ", test.name);

            if (suite.setup != 0) {
                suite.setup();
            }
            bool result = test.function();
            if (suite.cleanup != 0) {
                suite.cleanup();
            }

            if (result) {
                fmt_print("OK!\n");
                passed += 1;
            }
            else {
                fmt_print("FAIL!\n");
                failed += 1;
            }
        }
    }

    fmt_print("\n--------------------------------------------------\n");
    fmt_print("PASSED: %d, FAILED: %d\n", passed, failed);

    cpu_halt();
}
