#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define UNUSED(x) (void)(x)
#define PACKED __attribute__((packed))

#include "multiboot.c"
#include "gdt.c"
#include "paging.c"

#define PAGE_SIZE 4096

// Linker constants.
extern int __boot_start;
extern int __loaded_size;

// Kernel entry point.
void kernel_main(Multiboot_Info *boot_info, void *page_directory, void *page_table_area);



// @TODO: Move this to std?
void *align_forward(void *p, int alignment) {
    int p_int = (int)p;
    return (void*)(p_int + (alignment - p_int % alignment));
}

// @TODO: Move this to paging_init
typedef struct {
    bool page_directory_placed;
    uint32_t *page_directory;
    bool page_table_area_placed;
    void *page_table_area;
} Placement_Result;

Placement_Result place_page_metadata_in_section(uint64_t base_addr, uint64_t length, bool page_directory_placed) {
    Placement_Result result = {0};

    uint32_t *page_directory_placement = align_forward((void*)(size_t)base_addr, PAGE_SIZE);

    if (
        !page_directory_placed &&
        page_directory_placement + sizeof(*result.page_directory) < base_addr + length
    ) {
        result.page_directory_placed = true;
        result.page_directory = page_directory_placement;
        length -= (size_t)page_directory_placement + sizeof(*result.page_directory) - base_addr;
        base_addr = (size_t)page_directory_placement + sizeof(*result.page_directory);
    }

    // Find a place to put the page tables. There are 1024 page tables with 1024 4-byte entries each.
    void *page_table_area_placement = align_forward((void*)(size_t)base_addr, PAGE_SIZE);
    if (base_addr + length >= (size_t)page_table_area_placement + 1024 * 1024 * 4) {
        result.page_table_area_placed = true;
        result.page_table_area = page_table_area_placement;
    }

    return result;
}



void bootstrap(Multiboot_Info *boot_info) {
    gdt_init();

    // Scan for the first available segment large enough for the page directory and the first
    // available segment large enough for 1024 contiguous page tables.
    uint32_t* page_directory;
    void *page_table_area;
    bool page_directory_placed = false;
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
                Placement_Result placement = place_page_metadata_in_section(base_addr, length, page_directory_placed);
                if (!page_directory_placed && placement.page_directory_placed) {
                    page_directory = placement.page_directory;
                    page_directory_placed = true;
                }
                if (placement.page_table_area_placed) {
                    page_table_area = placement.page_table_area;
                    break;
                }

                base_addr = (uint64_t)(size_t)&__boot_start + (uint64_t)(size_t)&__loaded_size;
                length = segments->base_addr + segments->length - base_addr;
                placement = place_page_metadata_in_section(base_addr, length, page_directory_placed);
                if (!page_directory_placed && placement.page_directory_placed) {
                    page_directory = placement.page_directory;
                    page_directory_placed = true;
                }
                if (placement.page_table_area_placed) {
                    page_table_area = placement.page_table_area;
                    break;
                }
            }
            else {
                Placement_Result placement = place_page_metadata_in_section(segments->base_addr, segments->length, page_directory_placed);
                if (!page_directory_placed && placement.page_directory_placed) {
                    page_directory = placement.page_directory;
                    page_directory_placed = true;
                }
                if (placement.page_table_area_placed) {
                    page_table_area = placement.page_table_area;
                    break;
                }
            }
        }

        segments = (MMap_Segment*)((uint8_t*)segments + segments->size + 4);
    }

    paging_init(page_directory, page_table_area);

    kernel_main(boot_info, page_directory, page_table_area);
}
