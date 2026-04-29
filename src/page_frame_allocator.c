#define PAGE_SIZE 4096

uint32_t ALIGN(PAGE_SIZE) page_tables[1024][1024] = {0};

bool pfa_commit_page(void *virt_addr) {
    UNUSED(virt_addr);
    return false;
}
