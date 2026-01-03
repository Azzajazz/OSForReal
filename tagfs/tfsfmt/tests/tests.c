#include <stdbool.h>
#include <stdio.h>

typedef bool (*Test_Fn)();

typedef struct {
    char *name;
    Test_Fn function;
} Test;

#define TEST(test_name) \
    (Test){.name = #test_name, .function = test_name}

#define ARRAY_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))

bool dummy_test() {
    return true;
}

Test tests[] = {
    TEST(dummy_test),
};

int main() {
    int passed = 0;
    int failed = 0;

    for (size_t i = 0; i < ARRAY_LEN(tests); i++) {
        Test test = tests[i];
        printf("Running test %s... ", test.name);

        bool result = test.function();
        if (result) {
            printf("OK!\n");
            passed += 1;
        }
        else {
            printf("FAIL!\n");
            failed += 1;
        }

        printf("--------------------\n");
        printf("PASSED: %d, FAILED: %d\n", passed, failed);
    }
}
