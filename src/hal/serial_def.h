#define PORT 0x3f8          // COM1

bool serial_init();
bool serial_is_transmit_empty();
void serial_write(char a);
