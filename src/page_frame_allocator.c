// @TODO: Move paging.c to a place common to bootstrap and kernel.
#include "boot/paging.c"

extern uint32_t page_directory[1024];

#define RESERVED_PHYS 10 * MiB

uint32_t ALIGN(PAGE_SIZE) pfa_page_tables[254][1024] = {0};
// 32074 = (0x100000000 (virtual memory size)
//  - 8MiB (reserved memory for bootloader and kernel))
//  / 4096 (page size)
//  / 32 (bits per entry)
uint32_t pfa_page_bitmap[32704] = {0};

// @TODO: Mark some pages in the bitmap as used based on multiboot mmap info.
bool pfa_init(Multiboot_Info *boot_info) {
    UNUSED(boot_info);
    return true;
}

size_t mapped_virt_to_pfa_tables_index(void *virt_addr) {
    size_t page_virt_base = (size_t)__kernel_start + MAX_KERNEL_SIZE;
    size_t index = ((size_t)virt_addr - page_virt_base) / (PAGE_SIZE * 1024);
    ASSERT(index < ARRAY_LEN(pfa_page_tables), "Index too large.");
    return index;
}

void *pfa_tables_index_to_mapped_virt(size_t index) {
    ASSERT(index < ARRAY_LEN(pfa_page_tables), "Index too large.");
    size_t page_virt_base = (size_t)__kernel_start + MAX_KERNEL_SIZE;
    return (void *)(index * PAGE_SIZE * 1024 + page_virt_base);
}

Phys_Addr pfa_tables_index_to_table_phys(size_t index) {
    ASSERT(index < ARRAY_LEN(pfa_page_tables), "Index too large.");
    size_t pfa_table_virt = (size_t)&pfa_page_tables[index];
    ASSERT(
        pfa_table_virt >= (size_t)__kernel_start && pfa_table_virt < (size_t)__kernel_end,
        "Virtual address should be in the kernel virtual address space."
    );
    return (Phys_Addr){
        pfa_table_virt - ((uint32_t)__kernel_start - (uint32_t)__kernel_phys_start)
    };
}

size_t mapped_virt_to_page_directory_entry(void *virt) {
    return (size_t)virt >> 22;
}

size_t mapped_virt_to_page_table_entry(void *virt) {
    return ((size_t)virt >> 12) & 0x3FF;
}

bool pfa_commit_page(void *virt_addr) {
    size_t virt_addr_s = (size_t)virt_addr;
    ASSERT((virt_addr_s & 0xfff) == 0, "virt_addr is not page aligned.");

    size_t page_frame = 1;

    for (size_t i = 0; i < ARRAY_LEN(pfa_page_bitmap); i++) {
        if (pfa_page_bitmap[i] != 0xffffffff) {
            // This entry contains a free page.
            int zero_page_bit = __builtin_ctz(~pfa_page_bitmap[i]);
            pfa_page_bitmap[i] |= (1 << zero_page_bit);
            page_frame = (i * 32 + zero_page_bit) * PAGE_SIZE + RESERVED_PHYS;
            break;
        }
    }

    if (page_frame == 1) {
        return false;
    }
    ASSERT((page_frame & 0xFFF) == 0, "page_frame is not page aligned.");

    size_t page_directory_entry = mapped_virt_to_page_directory_entry(virt_addr);
    size_t page_table_entry = mapped_virt_to_page_table_entry(virt_addr);
    size_t pfa_page_tables_index = mapped_virt_to_pfa_tables_index(virt_addr);
    Phys_Addr page_table_phys = pfa_tables_index_to_table_phys(pfa_page_tables_index);
    page_directory[page_directory_entry] = page_table_phys.value |
        PAGE_DIR_PRESENT | PAGE_DIR_RW | PAGE_DIR_ACCESS_ALL | PAGE_DIR_ACCESSED;
    pfa_page_tables[pfa_page_tables_index][page_table_entry] = page_frame |
        PAGE_TABLE_PRESENT | PAGE_TABLE_RW | PAGE_TABLE_ACCESS_ALL | PAGE_TABLE_ACCESSED;

    return true;
}

// @NOTE: Used for testing, so that global state can be reset. This is an extremely uncommon and
// dangerous operation otherwise, since it uncommits physical pages of memory allocated by the virtual
// memory allocator without freeing the corresponding virtual memory.
void pfa_uncommit_all() {
    for (size_t i = 0; i < ARRAY_LEN(pfa_page_tables); i++) {
        for (size_t j = 0; j < ARRAY_LEN(pfa_page_tables[i]); j++) {
            size_t page_frame = pfa_page_tables[i][j] & ~0xFFF;
            size_t page_bitmap_index = (page_frame - RESERVED_PHYS) / PAGE_SIZE / 32;
            size_t page_bitmap_bit_index = (page_frame - RESERVED_PHYS) / PAGE_SIZE % 32;
            pfa_page_bitmap[page_bitmap_index] &= ~(1 << page_bitmap_bit_index);
        }

        memory_zero(&pfa_page_tables[i], sizeof(pfa_page_tables[i]));

        void *virt_addr = pfa_tables_index_to_mapped_virt(i);
        size_t page_directory_entry = (size_t)virt_addr >> 22;
        if ((page_directory[page_directory_entry] & PAGE_DIR_PRESENT) != 0) {
            memory_zero(&page_directory[page_directory_entry], sizeof(page_directory[page_directory_entry]));
        }
    }
}
