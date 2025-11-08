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

void terminal_putchar(char c) 
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
		terminal_putchar(str.data[i]);
}

void terminal_writestring(const char* data) 
{
    String str = str_literal(data);
	terminal_write(str);
}
