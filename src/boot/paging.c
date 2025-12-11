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
bool paging_init(Bootstrap_Info info, Multiboot_Info *boot_info) {
    // Set up the page directory.
    uint8_t page_directory_flags = PAGE_DIR_PRESENT | PAGE_DIR_RW | PAGE_DIR_ACCESS_ALL | PAGE_DIR_ACCESSED;
    for (int i = 0; i < 1024; i++) {
        uint32_t *page_directory_entry = info.page_directory + 4 * i;
        // @FIXME: Magic constant.
        uint32_t page_table_addr = (uint32_t)(info.page_tables + 1024 * 4 * i);
        *page_directory_entry = page_table_addr | page_directory_flags;
    }

    // Identity map everything.
    uint8_t page_table_flags = PAGE_TABLE_PRESENT | PAGE_TABLE_RW | PAGE_TABLE_ACCESS_ALL | PAGE_TABLE_ACCESSED;
    for (uint64_t page_u64 = 0; page_u64 <= 0xfffff000; page_u64 += PAGE_SIZE) {
        uint32_t page = (uint32_t)page_u64;
        map_page_to_virtual(page, page, page_table_flags, info.page_directory);
    }

    // Map the kernel to address 0xC0000000
    uint32_t kernel_virt_addr = (uint32_t)&__kernel_start;
    for (uint32_t page = (uint32_t)&__kernel_phys_start; page < (uint32_t)&__kernel_phys_end; page += PAGE_SIZE) {
        map_page_to_virtual(page, kernel_virt_addr, page_table_flags, info.page_directory);
        kernel_virt_addr += PAGE_SIZE;
    }

    // Initialize the page bitmap.
    size_t page_base_addr = 0;
    // @TODO: This should be kernel_phys_start, but for now we have to reserve the bootstrap section so that
    // we don't overwrite the gdt and stack. Eventually, we will move the gdt and stack somewhere else.
    size_t boot_start = (size_t)&__boot_start;
    size_t kernel_phys_end = (size_t)&__kernel_phys_end;
    for (size_t i = 0; i < info.page_bitmap_size; i += 4) {
        uint32_t *page_mask = (uint32_t*)(info.page_bitmap + i);

        for (size_t j = 0; j < 32; j++) {
            // Check if this page contains any of the metadata in the boot info, or the kernel.
            // If it does, then mark it occupied by setting the corresponding bit in the bitmap.
            //
            // NOTE: The kernel code, info.page_directory and info.page_tables are assumed to be page aligned.
            // Ideally we would assert this, but the assert mechanisms aren't available until we set up printing to VGA and
            // serial ports, which isn't done until we call into the kernel proper.

            // The page directory is page aligned and one page long, so just checking the starting address
            // of this page against the address of the page directory is sufficient.
            bool page_contains_page_directory = (size_t)info.page_directory == page_base_addr;

            // The page tables section is guaranteed to be at least one page long, so the case where the page table
            // section is strictly contained in this page is impossible. We only need to check whether the endpoints of
            // this page lie within the page tables section.
            bool page_contains_page_tables =
                ((size_t)info.page_tables <= page_base_addr && page_base_addr < (size_t)info.page_tables + PAGE_TABLES_SIZE) ||
                ((size_t)info.page_tables < page_base_addr + PAGE_SIZE && page_base_addr + PAGE_SIZE <= (size_t)info.page_tables + PAGE_TABLES_SIZE);

            // The kernel is guaranteed at least one page long, so the case where the kernel is strictly contained in this page is
            // impossible.
            bool page_contains_kernel =
                (boot_start <= page_base_addr && page_base_addr <= kernel_phys_end) ||
                (boot_start < page_base_addr + PAGE_SIZE && page_base_addr + PAGE_SIZE <= kernel_phys_end + 1);

            // The page table bitmap has no guarantees whatsoever.
            bool page_contains_page_bitmap =
                ((size_t)info.page_bitmap <= page_base_addr && page_base_addr < (size_t)info.page_bitmap + info.page_bitmap_size) ||
                ((size_t)info.page_bitmap < page_base_addr + PAGE_SIZE && page_base_addr + PAGE_SIZE <= (size_t)info.page_bitmap + info.page_bitmap_size) ||
                ((size_t)info.page_bitmap > page_base_addr && page_base_addr + PAGE_SIZE > (size_t)info.page_bitmap + info.page_bitmap_size);

            if (page_contains_page_directory || page_contains_page_tables || page_contains_kernel || page_contains_page_bitmap) {
                *page_mask |= (1 << j);
            }
            else {
                *page_mask &= ~(1 << j);
            }

            page_base_addr += PAGE_SIZE;
        }
    }

    // Any pages that aren't fully contained in a block of type 1 memory are marked as used.
    MMap_Segment *segment = (MMap_Segment*)boot_info->mmap_addr;
    size_t bytes_traversed = 0;
    while (bytes_traversed < boot_info->mmap_length) {
        if (segment->type != 1) {
            uint64_t addr = segment->base_addr;
            // Align backwards to PAGE_SIZE.
            if (addr % PAGE_SIZE != 0) {
                addr -= addr % PAGE_SIZE;
            }
            
            for(; addr < segment->base_addr + segment->length; addr += PAGE_SIZE) {
                int mask_offset = (addr / PAGE_SIZE) / 32;
                int bit_offset = (addr / PAGE_SIZE) % 32; 
                uint32_t* mask = (uint32_t*)info.page_bitmap + mask_offset;
                *mask |= (1 << bit_offset);
            }
        }

        bytes_traversed += segment->size + 4;
        segment = (MMap_Segment*)((uint8_t*)segment + segment->size + 4);
    }

    asm(
        "mov cr3, %0\t\n"

        "mov eax, cr0\t\n"
        "or eax, 0x80000001\t\n"
        "mov cr0, eax\t\n"
        :
        : "r" ((uint32_t)info.page_directory)
    );
    
    return true;
}
