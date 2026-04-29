#define PAGE_SIZE 4096

typedef struct Memory_Segment {
    struct Memory_Segment *next;
    size_t length;
} Memory_Segment;

Memory_Segment *free_segments_head;

bool memory_init() {
    // Map one page so that we have some memory. We reserve 4MiB for the page tables
    // and we allow 4MiB for the kernel to grow, so use the virtual address 0xC0800000.
    void *first_page_virt_addr = (void *)0xC0800000;
    bool success = pfa_commit_page(first_page_virt_addr);
    free_segments_head = first_page_virt_addr;
    free_segments_head->next = 0;
    free_segments_head->length = PAGE_SIZE;
    return success;
}

size_t align_forward(size_t size) {
    return (size + 31) & ~31;
}

void *memory_allocate(size_t size) {
    // Walk the free list.
    // If we get to somewhere that can fit the size, then use that.
    // Otherwise, append pages to the end of the last memory segment until it's
    // big enough.
    size_t true_size = align_forward(size);

    Memory_Segment *prev_segment = 0;
    Memory_Segment *segment = free_segments_head;
    while (segment != 0) {
        if (segment->length <= true_size) {
            Memory_Segment *remaining_segment = (Memory_Segment *)((uint8_t *)segment + true_size);
            if (prev_segment != 0) {
                prev_segment->next = remaining_segment;
            }
            remaining_segment->length = segment->length - true_size;
            return segment;
        }
        prev_segment = segment;
        segment = segment->next;
    }

    // If we get here, we don't have a big enough free segment. Allocate more pages.
    ASSERT(false, "TODO: Allocate more pages.");
    return 0;
}

void memory_free(void *address) {
    UNUSED(address);
}
