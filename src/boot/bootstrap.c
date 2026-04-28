#include "../common.h"

#define BOOT_FN __attribute__((section(".boot_text")))
#define BOOT_DATA __attribute__((section(".boot_data")))
#define BOOT_BSS __attribute__((section(".boot_bss")))

#define ALIGN(x) __attribute__((aligned(x)))

// Linker constants.
extern char __boot_start;
extern char __boot_end[];
extern char __kernel_start[];
extern char __kernel_end[];
extern char __kernel_phys_start[];
extern char __kernel_phys_end[];

#define PAGE_DIR_SIZE 4096
#define PAGE_TABLES_SIZE 1024 * 1024 * 4

#include "multiboot.c"
#include "gdt.c"
#include "paging.c"

BOOT_DATA ALIGN(PAGE_SIZE) uint32_t page_directory[1024] = {0};
BOOT_DATA ALIGN(PAGE_SIZE) uint32_t low_page_table[1024] = {0};
BOOT_DATA ALIGN(PAGE_SIZE) uint32_t kernel_page_table[1024] = {0};

#ifdef KERNEL_TEST
    // Kernel test runner.
    void kernel_test();
#else
    // Kernel entry point.
    void kernel_main(Multiboot_Info *boot_info);
#endif

void memory_copy(void *dst, void *src, size_t size) {
    uint8_t *dst_b = dst;
    uint8_t *src_b = src;
    for (size_t i = 0; i < size; i++) {
        dst_b[i] = src_b[i];
    }
}

BOOT_FN void bootstrap(Multiboot_Info *boot_info) {
    // Copy the multiboot info and mmap information to a known location before setting
    // up paging.
    Multiboot_Info *b_info = (Multiboot_Info *)__boot_end;
    memory_copy(b_info, boot_info, sizeof(*boot_info));
    uint8_t *new_mmap_addr = (uint8_t *)(b_info + 1);
    memory_copy(new_mmap_addr, (uint8_t *)b_info->mmap_addr, b_info->mmap_length);
    b_info->mmap_addr = (uint32_t)new_mmap_addr;

    // Identity map the first 2 MB and map the kernel to address 0xC0000000.
    uint8_t page_directory_flags = PAGE_DIR_PRESENT | PAGE_DIR_RW | PAGE_DIR_ACCESS_ALL | PAGE_DIR_ACCESSED;
    uint8_t page_table_flags = PAGE_TABLE_PRESENT | PAGE_TABLE_RW | PAGE_TABLE_ACCESS_ALL | PAGE_TABLE_ACCESSED;

    size_t virt_addr = 0;
    page_directory[virt_addr >> 22] = (uint32_t)low_page_table | page_directory_flags;
    for (size_t phys_addr = 0; phys_addr < 0x200000; phys_addr += PAGE_SIZE) {
        low_page_table[(virt_addr >> 12) & 0x3FF] = phys_addr | page_table_flags;
        virt_addr += PAGE_SIZE;
    }

    virt_addr = 0xC0000000;
    page_directory[virt_addr >> 22] = (uint32_t)kernel_page_table | page_directory_flags;
    for (size_t phys_addr = (size_t)__kernel_phys_start; phys_addr < (size_t)__kernel_phys_end; phys_addr += PAGE_SIZE) {
        kernel_page_table[(virt_addr >> 12) & 0x3FF] = phys_addr | page_table_flags;
        virt_addr += PAGE_SIZE;
    }

    // Initialize the paging.
    asm(
        "mov cr3, %0\t\n"

        "mov eax, cr0\t\n"
        "or eax, 0x80000001\t\n"
        "mov cr0, eax\t\n"
        :
        : "r" ((uint32_t)page_directory)
    );

#ifdef KERNEL_TEST
    kernel_test();
#else
    kernel_main(b_info);
#endif
}
