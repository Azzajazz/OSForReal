void put_char(char c) {
    terminal_put_char(c);
    serial_write(c);
}

void write(String str) 
{
	for (size_t i = 0; i < str.length; i++)
		put_char(str.data[i]);
}

void write_string(char *data) 
{
    String str = str_literal(data);
	write(str);
}

void put_uint(uint64_t n) {
    if (n == 0) {
        put_char('0');
        return;
    }

    // unsigned int is at most 2^64 - 1, which has 19 digits
    char digits[19];
    size_t num_digits = 0;

    // Digits will print in reverse order if we print them here.
    while (n > 0) {
        digits[num_digits] = n % 10;
        num_digits += 1;
        n /= 10;
    }

    // Print the digits the correct direction.
    for (size_t i = 0; i < num_digits; i++) {
        char c = digits[num_digits - i - 1] + '0';
        put_char(c);
    }
}

void put_hex(uint64_t n) {
    if (n == 0) {
        put_char('0');
        return;
    }

    // uint64_t is at most 0xffffffffffffffff, which has 16 digits.
    char digits[16];
    size_t num_digits = 0;

    // Digits will print in reverse order if we print them here.
    while (n > 0) {
        digits[num_digits] = n & 0xf;
        num_digits += 1;
        n >>= 4;
    }

    // Print the digits the correct direction.
    for (size_t i = 0; i < num_digits; i++) {
        char c = digits[num_digits - i - 1];
        if (c >= 0 && c <= 9) {
            put_char(c + '0');
        }
        else {
            put_char(c - 10 + 'A');
        }
    }
}

void put_int(int n) {
    if (n == 0) {
        put_char('0');
        return;
    }

    if (n < 0) {
        put_char('-');
        n = -n;
    }
    put_uint(n);
}



// --------------------------------------------------
// FORMAT PRINTING
// --------------------------------------------------

typedef enum {
    FMT_PRINT_DT_UNKNOWN,
    FMT_PRINT_DT_CHAR,
    FMT_PRINT_DT_UINT,
    FMT_PRINT_DT_INT,
    FMT_PRINT_DT_HEX,
    FMT_PRINT_DT_STRING,
} Fmt_Print_Data_Type;

typedef enum {
    FMT_PRINT_LM_NONE,
    FMT_PRINT_LM_L,
    FMT_PRINT_LM_H,
    FMT_PRINT_LM_HH,
} Fmt_Print_Length_Modifier;

typedef struct {
    Fmt_Print_Data_Type data_type;
    Fmt_Print_Length_Modifier length_modifier;
} Fmt_Print_Specifier;

Fmt_Print_Specifier _parse_specifier(char **fmt) {
    Fmt_Print_Specifier specifier = {0};
    for (;;) {
        // @FIXME: More complete length modifiers.
        switch (**fmt) {
            case 'h': {
                *fmt += 1;
                if (**fmt == 'h') {
                    specifier.length_modifier = FMT_PRINT_LM_HH;
                    *fmt += 1;
                }
                else {
                    specifier.length_modifier = FMT_PRINT_LM_H;
                }
            } break;

            case 'l':
                specifier.length_modifier = FMT_PRINT_LM_L;
                *fmt += 1;
                break;
        }

        switch (**fmt) {
            // @FIXME: Parse the rest of the specifiers.
            case 'c':
                specifier.data_type = FMT_PRINT_DT_CHAR;
                break;

            case 'u':
                specifier.data_type = FMT_PRINT_DT_UINT;
                break;

            case 'd':
                specifier.data_type = FMT_PRINT_DT_INT;
                break;

            case 'x':
                specifier.data_type = FMT_PRINT_DT_HEX;
                break;

            case 's':
                specifier.data_type = FMT_PRINT_DT_STRING;
                break;

            default:
                specifier.data_type = FMT_PRINT_DT_UNKNOWN;
                break;
        }
        *fmt += 1;
        break;
    }

    return specifier;
}

#define fmt_print(...) fmt_print_impl(__VA_ARGS__, 0)
void fmt_print_impl(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char *p_fmt = fmt;
    while (*p_fmt) {
        char c = *p_fmt;
        if (c == '%') {
            p_fmt += 1;
            Fmt_Print_Specifier specifier = _parse_specifier(&p_fmt);
            switch (specifier.data_type) {
                case FMT_PRINT_DT_CHAR: {
                    char next = va_arg(args, int);
                    put_char(next);
                } break;

                case FMT_PRINT_DT_UINT: {
                    switch (specifier.length_modifier) {
                        case FMT_PRINT_LM_NONE: {
                            uint32_t next = va_arg(args, uint32_t);
                            put_uint((uint64_t)next);
                        } break;

                        case FMT_PRINT_LM_L: {
                            uint64_t next = va_arg(args, uint64_t);
                            put_uint(next);
                        } break;

                        case FMT_PRINT_LM_H: {
                            uint16_t next = va_arg(args, uint32_t);
                            put_uint((uint64_t)next);
                        } break;
                        
                        case FMT_PRINT_LM_HH: {
                            uint8_t next = va_arg(args, uint32_t);
                            put_uint((uint64_t)next);
                        } break;
                    }
                } break;

                case FMT_PRINT_DT_INT: {
                    int next = va_arg(args, int);
                    put_int(next);
                } break;

                case FMT_PRINT_DT_HEX: {
                    switch (specifier.length_modifier) {
                        case FMT_PRINT_LM_NONE: {
                            uint32_t next = va_arg(args, uint32_t);
                            put_hex((uint64_t)next);
                        } break;

                        case FMT_PRINT_LM_L: {
                            uint64_t next = va_arg(args, uint64_t);
                            put_hex(next);
                        } break;

                        case FMT_PRINT_LM_H: {
                            uint16_t next = va_arg(args, uint32_t);
                            put_hex((uint64_t)next);
                        } break;
                        
                        case FMT_PRINT_LM_HH: {
                            uint8_t next = va_arg(args, uint32_t);
                            put_hex((uint64_t)next);
                        } break;
                    }
                } break;

                case FMT_PRINT_DT_STRING: {
                    char *next = va_arg(args, char*);
                    write_string(next);
                } break;
                
                default:
                    write_string("%(UNKNOWN_SPECIFIER)");
                    break;
            }
        }
        else {
            put_char(c);
            p_fmt += 1;
        }
    }

    int next = va_arg(args, int);
    while (next) {
        write_string(" %(EXTRA_ARGUMENT)");
        next = va_arg(args, int);
    }

    va_end(args);
}
