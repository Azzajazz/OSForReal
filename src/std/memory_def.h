bool memory_init();
size_t align_backward(size_t size, size_t alignment);
size_t align_forward(size_t size, size_t alignment);
void *memory_allocate(size_t size);
void memory_free(void *address);
void memory_fill(void *address, size_t length, uint8_t val);
void memory_zero(void *address, size_t length);
void memory_copy(void *dst, void *src, size_t size);
bool memory_compare(void *mem1, void *mem2, size_t size);
