#define PAGE_DIR_PRESENT (1 << 0)
#define PAGE_DIR_RW (1 << 1)
#define PAGE_DIR_ACCESS_ALL (1 << 2)
#define PAGE_DIR_WTCACHE (1 << 3)
#define PAGE_DIR_CACHE_DISABLE (1 << 4)
#define PAGE_DIR_ACCESSED (1 << 5)

#define PAGE_TABLE_PRESENT (1 << 0)
#define PAGE_TABLE_RW (1 << 1)
#define PAGE_TABLE_ACCESS_ALL (1 << 2)
#define PAGE_TABLE_WTCACHE (1 << 3)
#define PAGE_TABLE_CACHE_DISABLE (1 << 4)
#define PAGE_TABLE_ACCESSED (1 << 5)
#define PAGE_TABLE_DIRTY (1 << 6)
#define PAGE_TABLE_GLOBAL (1 << 8)

#define PAGE_SIZE 4096

// @TODO: Move this to std?
// @FIXME: Avoid modulo.
void *align_forward(void *p, int alignment) {
    int p_int = (int)p;
    if (p_int % alignment == 0) {
        return p;
    } else {
        return (void*)(p_int + (alignment - p_int % alignment));
    }
}

typedef struct {
    bool page_directory_placed;
    uint32_t *page_directory;
    bool page_table_area_placed;
    uint32_t *page_table_area;
} Page_Metadata_Placement;

// @FIXME: Overcomplicated?
Page_Metadata_Placement place_page_metadata_in_section(uint64_t base_addr, uint64_t length, bool page_directory_placed) {
    Page_Metadata_Placement result = {0};

    uint32_t *page_directory_placement = align_forward((void*)(size_t)base_addr, PAGE_SIZE);

    if (
        !page_directory_placed &&
        (size_t)(page_directory_placement + sizeof(*result.page_directory)) < base_addr + length
    ) {
        result.page_directory_placed = true;
        result.page_directory = page_directory_placement;
        length -= (size_t)page_directory_placement + sizeof(*result.page_directory) - base_addr;
        base_addr = (size_t)page_directory_placement + sizeof(*result.page_directory);
    }

    // Find a place to put the page tables. There are 1024 page tables with 1024 4-byte entries each.
    void *page_table_area_placement = align_forward((void*)(size_t)base_addr, PAGE_SIZE);
    // @FIXME: Magic constants
    if (base_addr + length >= (size_t)page_table_area_placement + 1024 * 1024 * 4) {
        result.page_table_area_placed = true;
        result.page_table_area = page_table_area_placement;
    }

    return result;
}

Page_Metadata_Placement place_page_metadata(Multiboot_Info *boot_info) {
    // Scan for the first available segment large enough for the page directory and the first
    // available segment large enough for 1024 contiguous page tables.
    Page_Metadata_Placement result = {0};

    MMap_Segment *segments = (MMap_Segment*)boot_info->mmap_addr;
    MMap_Segment *past_last_segment = (MMap_Segment*)(boot_info->mmap_addr + boot_info->mmap_length);
    while (segments < past_last_segment) {
        if (segments->type == 1) {
            if (
                segments->base_addr <= (uint64_t)(size_t)&__boot_start &&
                segments->base_addr + segments->length >= (uint64_t)(size_t)&__boot_start + (uint64_t)(size_t)&__loaded_size
            ) {
                // This is the segment with our kernel in it, so check the area above and below the kernel.
                uint64_t base_addr = segments->base_addr;
                uint64_t length = (uint64_t)(size_t)&__boot_start - segments->base_addr;
                Page_Metadata_Placement placement = place_page_metadata_in_section(base_addr, length, result.page_directory_placed);
                if (!result.page_directory_placed && placement.page_directory_placed) {
                    result.page_directory = placement.page_directory;
                    result.page_directory_placed = true;
                }
                if (placement.page_table_area_placed) {
                    result.page_table_area = placement.page_table_area;
                    result.page_table_area_placed = true;
                    break;
                }

                base_addr = (uint64_t)(size_t)&__boot_start + (uint64_t)(size_t)&__loaded_size;
                length = segments->base_addr + segments->length - base_addr;
                placement = place_page_metadata_in_section(base_addr, length, result.page_directory_placed);
                if (!result.page_directory_placed && placement.page_directory_placed) {
                    result.page_directory = placement.page_directory;
                    result.page_directory_placed = true;
                }
                if (placement.page_table_area_placed) {
                    result.page_table_area = placement.page_table_area;
                    result.page_table_area_placed = true;
                    break;
                }
            }
            else {
                Page_Metadata_Placement placement = place_page_metadata_in_section(segments->base_addr, segments->length, result.page_directory_placed);
                if (!result.page_directory_placed && placement.page_directory_placed) {
                    result.page_directory = placement.page_directory;
                    result.page_directory_placed = true;
                }
                if (placement.page_table_area_placed) {
                    result.page_table_area = placement.page_table_area;
                    result.page_table_area_placed = true;
                    break;
                }
            }
        }

        segments = (MMap_Segment*)((uint8_t*)segments + segments->size + 4);
    }

    return result;
}

// NOTE: Assumes that the page directory entry has been written to.
// NOTE: Assumes that page is PAGE_SIZE aligned.
void map_page_to_virtual(
    uint32_t page,
    uint32_t virtual,
    uint8_t flags,
    uint32_t *page_directory
) {
    uint32_t page_directory_index = virtual >> 22;
    uint32_t *page_directory_entry = page_directory + page_directory_index;

    uint32_t *page_table = (uint32_t*)(*page_directory_entry & 0xfffff000);
    uint32_t page_table_index = (virtual >> 12) & 0x3ff;
    uint32_t *page_table_entry = page_table + page_table_index;

    *page_table_entry = page | flags;
}

// NOTE: page_table_area is guaranteed to have enough space to hold 1024 page tables contiguously.
bool paging_init(Multiboot_Info *boot_info) {
    Page_Metadata_Placement placement = place_page_metadata(boot_info);

    if (!placement.page_directory_placed || !placement.page_table_area_placed) {
        return false;
    }

    // Identity map everything for now.

    // Set up the page directory.
    uint8_t page_directory_flags = PAGE_DIR_PRESENT | PAGE_DIR_RW | PAGE_DIR_ACCESS_ALL | PAGE_DIR_ACCESSED;
    for (int i = 0; i < 1024; i++) {
        uint32_t *page_directory_entry = placement.page_directory + i;
        // @FIXME: Magic constant.
        uint32_t page_table_addr = (uint32_t)(placement.page_table_area + 1024 * i);
        *page_directory_entry = page_table_addr | page_directory_flags;
    }

    // Now set up the page tables.
    uint8_t page_table_flags = PAGE_TABLE_PRESENT | PAGE_TABLE_RW | PAGE_TABLE_ACCESS_ALL | PAGE_TABLE_ACCESSED;
    for (uint64_t page_u64 = 0; page_u64 <= 0xfffff000; page_u64 += PAGE_SIZE) {
        uint32_t page = (uint32_t)page_u64;
        map_page_to_virtual(page, page, page_table_flags, placement.page_directory);
    }

    asm(
        "mov cr3, %0\t\n"

        "mov eax, cr0\t\n"
        "or eax, 0x80000001\t\n"
        "mov cr0, eax\t\n"
        :
        : "r" ((uint32_t)placement.page_directory)
    );
    
    return true;
}
