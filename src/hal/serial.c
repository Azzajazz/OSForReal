#define PORT 0x3f8          // COM1

static bool serial_init() {
   out_8(PORT + 1, 0x00);    // Disable all interrupts
   out_8(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   out_8(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   out_8(PORT + 1, 0x00);    //                  (hi byte)
   out_8(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   out_8(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   out_8(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   out_8(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   out_8(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty (i.e: not same byte as sent)
   if(in_8(PORT + 0) != 0xAE) {
      return false;
   }

   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   out_8(PORT + 4, 0x0F);
   return true;
}

bool serial_is_transmit_empty() {
   return in_8(PORT + 5) & 0x20;
}

void serial_write(char a) {
   while (serial_is_transmit_empty() == 0);

   out_8(PORT,a);
}
