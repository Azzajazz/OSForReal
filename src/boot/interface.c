typedef struct {
    void *page_directory;
    void *page_tables;
    void *page_bitmap;
    size_t page_bitmap_size;
} Bootstrap_Info;
