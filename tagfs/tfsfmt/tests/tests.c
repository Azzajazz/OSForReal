#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
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

size_t get_file_size(int fd) {
    struct stat fd_stat;
    fstat(fd, &fd_stat);
    return fd_stat.st_size;
}



// --------------------------------------------------
// FORMAT SUBCOMMAND TESTS
// --------------------------------------------------

bool tfsfmt_test_format_with_default_args() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);

    char *argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

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

    munmap(mapped_img, image_size);
    close(fd);

    return true;
}

bool tfsfmt_test_format_with_custom_args() {
    size_t image_size = 1024 * 1111;
    int fd = create_fs_image(image_name, image_size);

    char *argv[] = {
        "./build/tfsfmt", "format",
        "-sector-count", "1111",
        "-sector-size", "1024",
        "-fm-sectors", "20",
        "-tm-sectors", "5",
        "-tf-sectors", "3",
        "-fat-sectors", "3",
        image_name, 0};
    run_until_completion("./build/tfsfmt", argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    ASSERT(fs_meta->version == 1);
    ASSERT(fs_meta->sector_count == 1111);
    ASSERT(fs_meta->sector_size == 1024);
    ASSERT(fs_meta->file_meta_sector_count == 20);
    ASSERT(fs_meta->tag_meta_sector_count == 5);
    ASSERT(fs_meta->tag_file_sector_count == 3);
    ASSERT(fs_meta->fat_sector_count == 3);
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

    munmap(mapped_img, image_size);
    close(fd);

    return true;
}



// --------------------------------------------------
// WRITE-FILE SUBCOMMAND TESTS
// --------------------------------------------------

bool tfsfmt_test_write_file_less_than_one_sector() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *src_path = "tests/part_sector.txt";
    char *dst_file = "part_sector.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-file", "-src", src_path, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    ASSERT(fs_meta->free_file_id == 2);

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->id == 1);
    ASSERT(file_meta->first_data_sector == 0);
    ASSERT(file_meta->size == src_size);
    int cmp_result = strcmp(file_meta->name, dst_file);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 0xffff);
    ASSERT(fat[1] == 0);

    uint8_t *data = get_data(mapped_img, fs_meta);
    // less_than_one_sector contains "Hello, world!", which is 13 characters long
    cmp_result = memcmp(data, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src, src_size);
    close(src_fd);

    return true;
}

bool tfsfmt_test_write_file_exactly_one_sector() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *src_path = "tests/one_sector.txt";
    char *dst_file = "one_sector.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-file", "-src", src_path, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    ASSERT(fs_meta->free_file_id == 2);

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->id == 1);
    ASSERT(file_meta->first_data_sector == 0);
    ASSERT(file_meta->size == src_size);
    int cmp_result = strcmp(file_meta->name, dst_file);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 0xffff);
    ASSERT(fat[1] == 0);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src, src_size);
    close(src_fd);

    return true;
}

bool tfsfmt_test_write_file_many_sectors() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *src_path = "tests/many_sectors.txt";
    char *dst_file = "many_sectors.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-file", "-src", src_path, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    ASSERT(fs_meta->free_file_id == 2);

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->id == 1);
    ASSERT(file_meta->first_data_sector == 0);
    ASSERT(file_meta->size == src_size);
    int cmp_result = strcmp(file_meta->name, dst_file);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 1);
    ASSERT(fat[1] == 2);
    ASSERT(fat[2] == 0xffff);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src, src_size);
    close(src_fd);

    return true;
}

// --------------------------------------------------
// TEST ARRAY
// --------------------------------------------------

Test tests[] = {
    TEST(tfsfmt_test_format_with_default_args),
    TEST(tfsfmt_test_format_with_custom_args),
    TEST(tfsfmt_test_write_file_less_than_one_sector),
    TEST(tfsfmt_test_write_file_exactly_one_sector),
    TEST(tfsfmt_test_write_file_many_sectors),
};
