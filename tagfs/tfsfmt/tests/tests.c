#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

#include "../src/utils.c"

char *image_name = "test_tmp.img";

void run_until_completion(char *path, char **argv) {
    int pid = fork();
    if (pid == 0) {
        // Child process.
        execvp(path, argv);
    }
    else {
        waitpid(pid, NULL, 0);
    }
}

int create_fs_image(char *name, size_t size) {
    int fd = open(name, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    uint8_t dummy_data[1024];
    memset(dummy_data, 0xaa, 1024);

    size_t written = 0;
    while (written < size) {
        written += write(fd, dummy_data, 1024);
    }
    ftruncate(fd, size);

    return fd;
}

bool tfsfmt_test_format_with_default_args() {
    size_t image_size = 512000;
    int fd = create_fs_image(image_name, image_size);

    char *argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    ASSERT(fs_meta->version == 1);
    ASSERT(fs_meta->sector_count == 1000);
    ASSERT(fs_meta->sector_size == 512);
    ASSERT(fs_meta->file_meta_sector_count == 10);
    ASSERT(fs_meta->tag_meta_sector_count == 10);
    ASSERT(fs_meta->tag_file_sector_count == 10);
    ASSERT(fs_meta->fat_sector_count == 2);
    ASSERT(fs_meta->free_file_id == 1);
    ASSERT(fs_meta->free_tag_id == 1);

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->id == 0);

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta->id == 0);

    Tag_File_Entry *tag_file = get_tag_file_array(mapped_img, fs_meta);
    ASSERT(tag_file->tag_id == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    int num_fat_entries = fs_meta->fat_sector_count * fs_meta->sector_size / 2;
    for (int i = 0; i < num_fat_entries; i++) {
        ASSERT(fat[i] == 0);
    }

    msync(mapped_img, image_size, MS_SYNC);
    close(fd);

    return true;
}
