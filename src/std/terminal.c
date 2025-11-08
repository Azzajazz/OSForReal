size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void _terminal_increment_row(void) {
    terminal_row += 1;
    if (terminal_row >= VGA_HEIGHT) {
        // Scroll the screen up.
        for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                const size_t index = y * VGA_WIDTH + x;
                terminal_buffer[index] = terminal_buffer[index + VGA_WIDTH];
            }
        }

        // Fill the last row with spaces.
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }

        // Set the terminal row.
        terminal_row = VGA_HEIGHT - 1;
    }
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_put_char(char c) 
{
    if (c == '\n') {
        terminal_column = 0;
        _terminal_increment_row();
    }
    else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        terminal_column += 1;
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            _terminal_increment_row();
        }
    }
}

void terminal_write(String str) 
{
	for (size_t i = 0; i < str.length; i++)
		terminal_put_char(str.data[i]);
}

void terminal_write_string(char *data) 
{
    String str = str_literal(data);
	terminal_write(str);
}

void terminal_put_uint(unsigned int n) {
    if (n == 0) {
        terminal_put_char('0');
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
        terminal_put_char(c);
    }
}

void terminal_put_int(int n) {
    if (n == 0) {
        terminal_put_char('0');
        return;
    }

    if (n < 0) {
        terminal_put_char('-');
        n = -n;
    }
    terminal_put_uint(n);
}



// --------------------------------------------------
// FORMAT PRINTING
// --------------------------------------------------

typedef enum {
    FMT_PRINT_UNKNOWN,
    FMT_PRINT_CHAR,
    FMT_PRINT_UINT,
    FMT_PRINT_INT,
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

        default:
            specifier.data_type = FMT_PRINT_UNKNOWN;
            break;
        }
        *fmt += 1;
        break;
    }

    return specifier;
}

#define terminal_fmt_print(fmt, ...) terminal_fmt_print_impl(fmt, __VA_ARGS__, 0)
void terminal_fmt_print_impl(char *fmt, ...) {
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
                terminal_put_char(next);
            } break;

            case FMT_PRINT_UINT: {
                unsigned int next = va_arg(args, unsigned int);
                terminal_put_uint(next);
            } break;

            case FMT_PRINT_INT: {
                int next = va_arg(args, int);
                terminal_put_int(next);
            } break;
            
            default:
                terminal_write_string("%(UNKNOWN_SPECIFIER)");
                break;
            }
        }
        else {
            terminal_put_char(c);
            p_fmt += 1;
        }
    }

    va_end(args);
}
