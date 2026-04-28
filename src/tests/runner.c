#include "../kernel.c"

typedef bool (*Test_Fn)();

typedef struct {
    char *name;
    Test_Fn function;
} Test;

#define TEST(test_name) \
    (Test){.name = #test_name, .function = test_name}

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

void kernel_test() {
    kernel_init();

    int passed = 0;
    int failed = 0;
    UNUSED(passed);
    UNUSED(failed);

    /*
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
    */

    cpu_halt();
}
