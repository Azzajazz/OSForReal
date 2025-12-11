#include "../common.h"

// Linker constants.
extern int __boot_start;
extern int __kernel_start;
extern int __kernel_end;
extern int __kernel_phys_start;
extern int __kernel_phys_end;
extern int __loaded_size;

#define PAGE_DIR_SIZE 1024 * 4
#define PAGE_TABLES_SIZE 1024 * 1024 * 4

#include "interface.c"
#include "multiboot.c"
#include "gdt.c"
#include "paging.c"

#ifdef KERNEL_TEST
    // Kernel test runner.
    void kernel_test(Bootstrap_Info info);
#else
    // Kernel entry point.
    void kernel_main(Multiboot_Info *boot_info, Bootstrap_Info info);
#endif

void place_page_metadata(
    size_t base_addr, size_t length,
    Bootstrap_Info *info,
    size_t page_directory_size, size_t page_tables_size, size_t page_bitmap_size,
    bool *page_directory_placed, bool *page_tables_placed, bool *page_bitmap_placed
) {
    // @TODO: Alignment?
    if (!*page_directory_placed) {
        if (base_addr + page_directory_size <= length) {
            info->page_directory = (void*)base_addr;
            base_addr += page_directory_size;
            length -= page_directory_size;
            *page_directory_placed = true;
        }
    }

    if (!*page_tables_placed) {
        if (base_addr + page_tables_size <= length) {
            info->page_tables = (void*)base_addr;
            base_addr += page_tables_size;
            length -= page_tables_size;
            *page_tables_placed = true;
        }
    }

    if (!*page_bitmap_placed) {
        if (base_addr + page_bitmap_size <= length) {
            info->page_bitmap = (void*)base_addr;
            base_addr += page_bitmap_size;
            length -= page_bitmap_size;
            *page_bitmap_placed = true;
        }
    }
}

void bootstrap(Multiboot_Info *boot_info) {
    size_t boot_start = (size_t)&__boot_start;
    size_t kernel_phys_end = (size_t)&__kernel_phys_end;
    // Find places to put the page directory, page tables and free page frame bitmap.
    // @Improvement: Right now we just place things in the first place they can go. To reduce fragmentation,
    // we could try to fill up smaller memory blocks as much as possible.

    Bootstrap_Info info;


    bool page_directory_placed = false;
    bool page_tables_placed = false;
    bool page_bitmap_placed = false;
    
    // Calculate the size of the bitmap.
    size_t memory_size = 0;

    size_t bytes_traversed = 0;
    MMap_Segment *segment = (MMap_Segment*)boot_info->mmap_addr;
    while (bytes_traversed < boot_info->mmap_length) {
        memory_size += (size_t)segment->length;

        bytes_traversed += segment->size + 4;
        segment = (MMap_Segment*)((uint8_t*)segment + segment->size + 4);
    }
    // 1 bit per page.
    size_t page_count = DIV_CEIL(memory_size, PAGE_SIZE);
    info.page_bitmap_size = DIV_CEIL(page_count, 8);

    // Place things.
    bytes_traversed = 0;
    segment = (MMap_Segment*)boot_info->mmap_addr;
    while (bytes_traversed < boot_info->mmap_length) {
        if (segment->type != 1) {
            bytes_traversed += segment->size + 4;
            segment = (MMap_Segment*)((uint8_t*)segment + segment->size + 4);
            continue;
        }

        // The kernel could be placed here. Make sure we don't overwrite it.
        bool contains_kernel = false;
        if (segment->base_addr >= boot_start && segment->base_addr + segment->length < kernel_phys_end) {
            contains_kernel = true;
        }

        if (segment->base_addr < boot_start && segment->base_addr + segment->length > boot_start) {
            // The start of the segment is free.
            size_t base_addr = segment->base_addr;
            size_t length = boot_start - segment->base_addr;
            place_page_metadata(
                base_addr, length,
                &info,
                PAGE_DIR_SIZE, PAGE_TABLES_SIZE, info.page_bitmap_size,
                &page_directory_placed, &page_tables_placed, &page_bitmap_placed
            );
            contains_kernel = true;
        }

        if (segment->base_addr < kernel_phys_end && segment->base_addr + segment->length > kernel_phys_end) {
            // The end of the segment is free.
            size_t base_addr = kernel_phys_end;
            size_t length = segment->base_addr + segment->length - kernel_phys_end;
            place_page_metadata(
                base_addr, length,
                &info,
                PAGE_DIR_SIZE, PAGE_TABLES_SIZE, info.page_bitmap_size,
                &page_directory_placed, &page_tables_placed, &page_bitmap_placed
            );
            contains_kernel = true;
        }

        if (!contains_kernel) {
            size_t base_addr = (size_t)segment->base_addr;
            size_t length = (size_t)segment->length;

            place_page_metadata(
                base_addr, length,
                &info,
                PAGE_DIR_SIZE, PAGE_TABLES_SIZE, info.page_bitmap_size,
                &page_directory_placed, &page_tables_placed, &page_bitmap_placed
            );
        }

        if (page_directory_placed && page_tables_placed && page_bitmap_placed) {
            break;
        }

        bytes_traversed += segment->size + 4;
        segment = (MMap_Segment*)((uint8_t*)segment + segment->size + 4);
    }

    // Initialize the paging.
    paging_init(info, boot_info);

#ifdef KERNEL_TEST
    kernel_test(info);
#else
    kernel_main(boot_info, info);
#endif
}
