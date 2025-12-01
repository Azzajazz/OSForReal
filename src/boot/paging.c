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
bool paging_init(Bootstrap_Info info) {
    // Identity map everything for now.

    // Set up the page directory.
    uint8_t page_directory_flags = PAGE_DIR_PRESENT | PAGE_DIR_RW | PAGE_DIR_ACCESS_ALL | PAGE_DIR_ACCESSED;
    for (int i = 0; i < 1024; i++) {
        uint32_t *page_directory_entry = info.page_directory + 4 * i;
        // @FIXME: Magic constant.
        uint32_t page_table_addr = (uint32_t)(info.page_tables + 1024 * 4 * i);
        *page_directory_entry = page_table_addr | page_directory_flags;
    }

    // Now set up the page tables.
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
