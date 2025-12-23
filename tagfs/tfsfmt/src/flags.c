#include <stdbool.h>
#include <assert.h>

typedef enum {
    FLAG_CSTR,
    FLAG_INT,
    FLAG_BOOL,
} Flag_Type;

typedef struct {
    char **value;
} Flag_Cstr;

typedef struct {
    int *value;
} Flag_Int;

typedef struct {
    bool *value;
} Flag_Bool;

typedef struct {
    Flag_Type type;
    char *option;
    bool provided;
    union {
        Flag_Cstr flag_cstr;
        Flag_Int flag_int;
        Flag_Bool flag_bool;
    };
} Flag;

#define FLAG_COUNT 128
static Flag flags[FLAG_COUNT] = {0};
static int flag_index;

#define POSITIONAL_COUNT 128
static Flag positionals[POSITIONAL_COUNT] = {0};
static int positional_index;

void flags_add_cstr_flag(char **value, char *option) {
    assert(flag_index < FLAG_COUNT);
    Flag *flag = &flags[flag_index];
    flag_index++;

    flag->type = FLAG_CSTR;
    flag->option = option;
    flag->provided = false;
    flag->flag_cstr.value = value;
}

void flags_add_int_flag(int *value, char *option) {
    assert(flag_index < FLAG_COUNT);
    Flag *flag = &flags[flag_index];
    flag_index++;

    flag->type = FLAG_INT;
    flag->option = option;
    flag->provided = false;
    flag->flag_int.value = value;
}

void flags_add_bool_flag(bool *value, char *option) {
    assert(flag_index < FLAG_COUNT);
    Flag *flag = &flags[flag_index];
    flag_index++;

    flag->type = FLAG_BOOL;
    flag->option = option;
    flag->provided = false;
    flag->flag_bool.value = value;
}

void flags_add_cstr_positional(char **value) {
    assert(positional_index < POSITIONAL_COUNT);
    Flag *flag = &positionals[positional_index];
    positional_index++;

    flag->type = FLAG_CSTR;
    flag->flag_cstr.value = value;
}

char *shift_args(int *argc, char ***argv) {
    char *arg = **argv;
    *argc = *argc - 1;
    *argv = *argv + 1;
    return arg;
}

bool flags_parse_flag(Flag *flag, int *argc, char ***argv) {
    flag->provided = true;

    switch (flag->type) {
    case FLAG_CSTR: {
        char *arg = shift_args(argc, argv);
        *flag->flag_cstr.value = arg;
    } break;
    case FLAG_INT: {
        char *arg = shift_args(argc, argv);
        // @TODO: Verify that this is a valid int.
        *flag->flag_int.value = atoi(arg);
    } break;
    case FLAG_BOOL: {
        *flag->flag_bool.value = true;
    } break;
    }

    return true;
}

// @TODO: Support positional arguments.
bool flags_parse_flags(int argc, char **argv) {
    int positional_i = 0;

    while (argc > 0) {
        char *arg = shift_args(&argc, &argv);
        
        if (*arg != '-') {
            if (positional_i >= positional_index) {
                return false;
            }

            positionals[positional_i].provided = true;
            *positionals[positional_i].flag_cstr.value = arg;
            positional_i++;
        }
        else {
            // Check flags.
            bool flag_found = false;
            for (int j = 0; j < flag_index; j++) {
                Flag *flag = &flags[j];

                if (strcmp(arg, flag->option) == 0) {
                    if (!flags_parse_flag(flag, &argc, &argv)) {
                        return false;
                    }
                    flag_found = true;
                    break;
                }
            }

            if (!flag_found) {
                return false;
            }
        }
    }

    return true;
}
