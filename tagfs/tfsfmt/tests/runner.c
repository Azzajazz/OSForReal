#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define ARRAY_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))

bool assert_impl(char *file, int line, char *cond_str, bool cond) {
    if (!(cond)) { \
        printf("\nASSERTION FAILED\n");
        printf("%s:%d: %s is false.\n", file, line, cond_str);
        return false;
    }

    return true;
}

#define ASSERT(cond) \
    if (!assert_impl(__FILE__, __LINE__, #cond, cond)) return false

#define TEST(test_name) \
    (Test){.name = #test_name, .function = test_name}

typedef bool (*Test_Fn)();

typedef struct {
    char *name;
    Test_Fn function;
} Test;

#include "tests.c"

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
    }

    printf("--------------------\n");
    printf("PASSED: %d, FAILED: %d\n", passed, failed);
}
