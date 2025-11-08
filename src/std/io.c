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

void put_uint(unsigned int n) {
    if (n == 0) {
        put_char('0');
        return;
    }

    // unsigned int is at most 2^32 - 1 = 4294967295, which has 10 digits.
    char digits[10];
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

void put_hex(unsigned int n) {
    if (n == 0) {
        put_char('0');
        return;
    }

    // unsigned int is at most 0xffffffff, which has 8 digits.
    char digits[8];
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
    FMT_PRINT_UNKNOWN,
    FMT_PRINT_CHAR,
    FMT_PRINT_UINT,
    FMT_PRINT_INT,
    FMT_PRINT_HEX,
    FMT_PRINT_STRING,
} Fmt_Print_Data_Type;

typedef struct {
    Fmt_Print_Data_Type data_type;
} Fmt_Print_Specifier;

Fmt_Print_Specifier _parse_specifier(char **fmt) {
    Fmt_Print_Specifier specifier = {0};
    for (;;) {
        switch (**fmt) {
        // @TODO: Parse the rest of the specifiers.
        case 'c':
            specifier.data_type = FMT_PRINT_CHAR;
            break;

        case 'u':
            specifier.data_type = FMT_PRINT_UINT;
            break;

        case 'd':
            specifier.data_type = FMT_PRINT_INT;
            break;

        case 'x':
            specifier.data_type = FMT_PRINT_HEX;
            break;

        case 's':
            specifier.data_type = FMT_PRINT_STRING;
            break;

        default:
            specifier.data_type = FMT_PRINT_UNKNOWN;
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
            case FMT_PRINT_CHAR: {
                char next = va_arg(args, int);
                put_char(next);
            } break;

            case FMT_PRINT_UINT: {
                unsigned int next = va_arg(args, unsigned int);
                put_uint(next);
            } break;

            case FMT_PRINT_INT: {
                int next = va_arg(args, int);
                put_int(next);
            } break;

            case FMT_PRINT_HEX: {
                unsigned int next = va_arg(args, unsigned int);
                put_hex(next);
            } break;

            case FMT_PRINT_STRING: {
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
