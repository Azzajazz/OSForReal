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

// NOTE: page_table_area is guaranteed to have enough space to hold 1024 page tables contiguously.
void paging_init(uint32_t *page_directory, void *page_table_area) {
    
}
