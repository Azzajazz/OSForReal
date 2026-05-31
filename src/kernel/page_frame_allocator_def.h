typedef enum {
    PP_KERNEL,
    PP_USER,
} Page_Privilege;

bool pfa_init(Multiboot_Info *boot_info);
bool pfa_commit_page(void *virt_addr, Page_Privilege privilege);
void pfa_uncommit_all();
