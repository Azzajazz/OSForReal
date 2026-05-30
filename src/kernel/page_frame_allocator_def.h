bool pfa_init(Multiboot_Info *boot_info);
bool pfa_commit_page(void *virt_addr);
void pfa_uncommit_all();
