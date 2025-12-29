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
    char *description;
    bool provided;
    bool required;
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

void flags_add_cstr_flag(char **value, char *option, char *description, bool required) {
    assert(flag_index < FLAG_COUNT);
    Flag *flag = &flags[flag_index];
    flag_index++;

    flag->type = FLAG_CSTR;
    flag->option = option;
    flag->description = description;
    flag->provided = false;
    flag->required = required;
    flag->flag_cstr.value = value;
}

void flags_add_int_flag(int *value, char *option, char *description, bool required) {
    assert(flag_index < FLAG_COUNT);
    Flag *flag = &flags[flag_index];
    flag_index++;

    flag->type = FLAG_INT;
    flag->option = option;
    flag->description = description;
    flag->provided = false;
    flag->required = required;
    flag->flag_int.value = value;
}

void flags_add_bool_flag(bool *value, char *option, char *description, bool required) {
    assert(flag_index < FLAG_COUNT);
    Flag *flag = &flags[flag_index];
    flag_index++;

    flag->type = FLAG_BOOL;
    flag->option = option;
    flag->description = description;
    flag->provided = false;
    flag->required = required;
    flag->flag_bool.value = value;
}

void flags_add_cstr_positional(char **value, char *name, char *description) {
    assert(positional_index < POSITIONAL_COUNT);
    Flag *flag = &positionals[positional_index];
    positional_index++;

    flag->type = FLAG_CSTR;
    flag->option = name;
    flag->description = description;
    flag->required = true;
    flag->flag_cstr.value = value;
}

char *shift_args(int *argc, char ***argv) {
    char *arg = **argv;
    *argc = *argc - 1;
    *argv = *argv + 1;
    return arg;
}

bool flags_parse_flag(Flag *flag, char *arg) {
    flag->provided = true;

    switch (flag->type) {
        case FLAG_CSTR: {
            *flag->flag_cstr.value = arg;
        } break;

        case FLAG_INT: {
            char *arg_end;
            int value = strtol(arg, &arg_end, 10);
            if (errno == ERANGE) {
                printf("%s %s: Integer out of range.\n\n", flag->option, arg);
                return false;
            }
            if (*arg_end != '\0') {
                printf("%s %s: Not a valid integer.\n\n", flag->option, arg);
                return false;
            }

            *flag->flag_int.value = value;
        } break;

        case FLAG_BOOL: {
            *flag->flag_bool.value = true;
        } break;
    }

    return true;
}

bool flags_parse_flags(int argc, char **argv) {
    int positional_i = 0;

    while (argc > 0) {
        char *arg = shift_args(&argc, &argv);
        
        if (*arg != '-') {
            if (positional_i > positional_index) {
                printf("Too many positional arguments provided (expected at most %d).\n\n", positional_index);
                return false;
            }

            if (!flags_parse_flag(&positionals[positional_i], arg)) {
                return false;
            }
            positional_i++;
        }
        else {
            // Check flags.
            bool flag_found = false;
            for (int j = 0; j < flag_index; j++) {
                Flag *flag = &flags[j];

                if (strcmp(arg, flag->option) == 0) {
                    char *arg = shift_args(&argc, &argv);
                    if (!flags_parse_flag(flag, arg)) {
                        return false;
                    }
                    flag_found = true;
                    break;
                }
            }

            if (!flag_found) {
                printf("%s: Unknown flag.\n\n", arg);
                return false;
            }
        }
    }

    for (int i = 0; i < positional_index; i++) {
        Flag positional = positionals[i];
        if (positional.required && !positional.provided) {
            printf("Argument <%s> is required, but was not given.\n\n", positional.option);
            return false;
        }
    }

    for (int i = 0; i < flag_index; i++) {
        Flag flag = flags[i];
        if (flag.required && !flag.provided) {
            printf("Option %s is required, but was not given.\n\n", flag.option);
            return false;
        }
    }

    return true;
}

void flags_print_help(char *program_name) {
    printf("Usage: %s", program_name);
    for (int i = 0; i < positional_index; i++) {
        Flag positional = positionals[i];
        printf(" <%s>", positional.option);
    }
    printf(" [OPTIONS]\n\n");

    printf("Positional arguments:\n");
    for (int i = 0; i < positional_index; i++) {
        Flag positional = positionals[i];
        printf("  %s: %s\n", positional.option, positional.description);
    }

    printf("\nOPTIONS:\n");
    for (int i = 0; i < flag_index; i++) {
        Flag flag = flags[i];
        printf("  %s", flag.option);
        if (flag.required) {
            printf(" (required)");
        }
        printf(": %s\n", flag.description);
    }
}
