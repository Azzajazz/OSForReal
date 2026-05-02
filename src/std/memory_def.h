bool memory_init();
size_t align_forward(size_t size);
void *memory_allocate(size_t size);
void memory_free(void *address);
void memory_fill(void *address, size_t length, uint8_t val);
void memory_zero(void *address, size_t length);
