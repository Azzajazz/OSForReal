typedef struct Memory_Segment {
    struct Memory_Segment *next;
    size_t length;
} Memory_Segment;

Memory_Segment *free_segments_head = 0;
void *next_page_virt_addr = 0;

bool pfa_commit_page(void *virt_addr);

bool memory_init() {
    next_page_virt_addr = (void *)(__kernel_start + MAX_KERNEL_SIZE);
    return true;
}

size_t align_forward(size_t size) {
    return (size + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);
}

void *memory_allocate(size_t size) {
    size_t true_size = align_forward(size + sizeof(size_t));

    if (free_segments_head == 0) {
        bool success = pfa_commit_page(next_page_virt_addr);
        if (!success) return 0;
        free_segments_head = next_page_virt_addr;
        free_segments_head->next = 0;
        free_segments_head->length = PAGE_SIZE;
        next_page_virt_addr += PAGE_SIZE;
    }

    Memory_Segment *prev_prev_segment = 0;
    Memory_Segment *prev_segment = 0;
    Memory_Segment *segment = free_segments_head;
    bool segment_can_be_split = false;
    while (segment != 0) {
        if (true_size + sizeof(Memory_Segment) <= segment->length) {
            segment_can_be_split = true;
            break;
        }
        else if (true_size <= segment->length) {
            segment_can_be_split = false;
            break;
        }

        prev_prev_segment = prev_segment;
        prev_segment = segment;
        segment = segment->next;
    }

    if (segment == 0) {
        // If we get here, we don't have a big enough free segment. Allocate more pages.
        ASSERT(prev_segment != 0, "prev_segment should be non-zero.");
        size_t last_segment_length = prev_segment->length;
        while (last_segment_length < true_size) {
            bool success = pfa_commit_page(next_page_virt_addr);
            if (!success) return 0;
            next_page_virt_addr += PAGE_SIZE;
            last_segment_length += PAGE_SIZE;
        }

        prev_segment->length = last_segment_length;
        segment = prev_segment;
        prev_segment = prev_prev_segment;

        segment_can_be_split = true_size + sizeof(Memory_Segment) <= segment->length;
    }

    if (segment_can_be_split) {
        Memory_Segment *remaining_segment = (Memory_Segment *)((uint8_t *)segment + true_size);
        if (prev_segment != 0) {
            prev_segment->next = remaining_segment;
        }
        remaining_segment->next = segment->next;
        remaining_segment->length = segment->length - true_size;
        if (segment == free_segments_head) {
            free_segments_head = remaining_segment;
        }
    }
    else {
        if (prev_segment != 0) {
            prev_segment->next = segment->next;
        }
        if (segment == free_segments_head) {
            free_segments_head = segment->next;
        }
    }

    *(size_t *)segment = size + sizeof(size_t);
    return (void *)segment + sizeof(size_t);
}

void memory_free(void *address) {
    if (free_segments_head == 0) {
        return;
    }

    Memory_Segment *segment = address - sizeof(size_t);

    Memory_Segment *prev_prev_segment = 0;
    Memory_Segment *prev_segment = 0;
    Memory_Segment *next_segment = 0;
    Memory_Segment *next_next_segment = free_segments_head;
    while (next_next_segment != 0) {
        ASSERT(next_segment != segment, "segment should not be in the free segments list.");
        ASSERT(next_next_segment != segment, "segment should not be in the free segments list.");
        if (next_next_segment > segment && next_segment < segment) {
            break;
        }

        prev_prev_segment = prev_segment;
        prev_segment = next_segment;
        next_segment = next_next_segment;
        next_next_segment = next_next_segment->next;
    }

    while (next_segment < segment) {
        prev_prev_segment = prev_segment;
        prev_segment = next_segment;
        next_segment = next_next_segment;
        if (next_next_segment != 0) {
            next_next_segment = next_next_segment->next;
        }
    }

    ASSERT(
        (prev_segment < segment && next_segment > segment) ||
        (prev_segment < segment && next_segment == 0) ||
        (prev_segment == 0 && next_segment > segment),
        "Free segments are not adjacent to segment."
    );

    segment->length = *(size_t *)segment;

    if ((uint8_t *)segment + segment->length == (uint8_t *)next_segment) {
        ASSERT(next_segment != 0, "next_segment does not exist.");
        segment->length += next_segment->length;        
        next_segment = next_next_segment;
    }
    if (prev_segment != 0 && (uint8_t *)prev_segment + prev_segment->length == (uint8_t *)segment) {
        prev_segment->length += segment->length;
        segment = prev_segment;
        prev_segment = prev_prev_segment;
    }

    if (free_segments_head > segment) {
        free_segments_head = segment;
    }

    prev_segment->next = segment;
    segment->next = next_segment;
}

// @NOTE: Used for tests only. I can't think of a single reason you'd want
// to do this otherwise.
void memory_free_all() {
    pfa_uncommit_all();
    next_page_virt_addr = (void *)__kernel_start + MAX_KERNEL_SIZE;
    free_segments_head = 0;
}

void memory_fill(void *address, size_t length, uint8_t val) {
    for (size_t i = 0; i < length; i++) {
        ((uint8_t *)address)[i] = val;
    }
}

void memory_zero(void *address, size_t length) {
    memory_fill(address, length, 0);
}
