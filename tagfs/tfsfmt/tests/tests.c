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

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->first_data_sector == 0);

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta->first_data_sector == 0);

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

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->first_data_sector == 0);

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta->first_data_sector == 0);

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
// WRITE-FILES SUBCOMMAND TESTS
// --------------------------------------------------

bool tfsfmt_test_write_files_less_than_one_sector() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *src_path = "tests/part_sector.txt";
    char *dst_file = "part_sector.txt";
    char *file_spec = "tests/part_sector.txt:part_sector.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-files", "-file", file_spec, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->first_data_sector == 1);
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

bool tfsfmt_test_write_files_exactly_one_sector() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *src_path = "tests/one_sector.txt";
    char *dst_file = "one_sector.txt";
    char *file_spec = "tests/one_sector.txt:one_sector.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-files", "-file", file_spec, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->first_data_sector == 1);
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

bool tfsfmt_test_write_files_many_sectors() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *src_path = "tests/many_sectors.txt";
    char *dst_file = "many_sectors.txt";
    char *file_spec = "tests/many_sectors.txt:many_sectors.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-files", "-file", file_spec, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->first_data_sector == 1);
    ASSERT(file_meta->size == src_size);
    int cmp_result = strcmp(file_meta->name, dst_file);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 1);
    ASSERT(fat[1] == 2);
    ASSERT(fat[2] == 0xffff);
    ASSERT(fat[3] == 0);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src, src_size);
    close(src_fd);

    return true;
}

bool tfsfmt_test_write_files_two_files() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);

    char *src_path_1 = "tests/many_sectors.txt";
    char *dst_file_1 = "many_sectors.txt";
    char *file_spec_1 = "tests/many_sectors.txt:many_sectors.txt";
    int src_fd_1 = open(src_path_1, O_RDONLY);
    size_t src_size_1 = get_file_size(src_fd_1);
    uint8_t *mapped_src_1 = mmap(NULL, src_size_1, PROT_READ, MAP_SHARED, src_fd_1, 0);

    char *src_path_2 = "tests/part_sector.txt";
    char *dst_file_2 = "part_sector.txt";
    char *file_spec_2 = "tests/part_sector.txt:part_sector.txt";
    int src_fd_2 = open(src_path_2, O_RDONLY);
    size_t src_size_2 = get_file_size(src_fd_2);
    uint8_t *mapped_src_2 = mmap(NULL, src_size_2, PROT_READ, MAP_SHARED, src_fd_2, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-files", "-file", file_spec_1, "-file", file_spec_2, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    // Check data for many_sectors.txt
    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta[0].first_data_sector == 1);
    ASSERT(file_meta[0].size == src_size_1);
    int cmp_result = strcmp(file_meta[0].name, dst_file_1);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 1);
    ASSERT(fat[1] == 2);
    ASSERT(fat[2] == 0xffff);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src_1, src_size_1);
    ASSERT(cmp_result == 0);

    // Check data for part_sector.txt
    ASSERT(file_meta[1].first_data_sector == 4);
    ASSERT(file_meta[1].size == src_size_2);
    cmp_result = strcmp(file_meta[1].name, dst_file_2);
    ASSERT(cmp_result == 0);

    ASSERT(fat[3] == 0xffff);
    ASSERT(fat[4] == 0);

    cmp_result = memcmp(data + fs_meta->sector_size * 3, mapped_src_2, src_size_2);
    ASSERT(cmp_result == 0);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src_1, src_size_1);
    close(src_fd_1);
    munmap(mapped_src_2, src_size_2);
    close(src_fd_2);

    return true;
}

bool tfsfmt_test_write_files_same_file() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);

    char *src_path = "tests/part_sector.txt";
    char *dst_file = "part_sector.txt";
    char *file_spec = "tests/part_sector.txt:part_sector.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {"./build/tfsfmt", "write-files", "-file", file_spec, "-file", file_spec, image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    // Check data for first part_sector.txt
    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta[0].first_data_sector == 1);
    ASSERT(file_meta[0].size == src_size);
    int cmp_result = strcmp(file_meta[0].name, dst_file);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 0xffff);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    // Check data for second part_sector.txt
    ASSERT(file_meta[1].first_data_sector == 2);
    ASSERT(file_meta[1].size == src_size);
    cmp_result = strcmp(file_meta[1].name, dst_file);
    ASSERT(cmp_result == 0);

    ASSERT(fat[1] == 0xffff);
    ASSERT(fat[2] == 0);

    cmp_result = memcmp(data + fs_meta->sector_size, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src, src_size);
    close(src_fd);

    return true;
}

bool tfsfmt_test_write_files_one_file_one_tag() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *src_path = "tests/part_sector.txt";
    char *dst_file = "part_sector.txt";
    char *file_spec = "tests/part_sector.txt:part_sector.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);
    char *tag_name = "test_tag";

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {
        "./build/tfsfmt", "write-files",
        "-file", file_spec,
        "-tag", tag_name,
        image_name, 0};
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->first_data_sector == 1);
    ASSERT(file_meta->size == src_size);
    int cmp_result = strcmp(file_meta->name, dst_file);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 0xffff);
    ASSERT(fat[1] == 0);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta->first_data_sector == 1); // @TODO: May not need this.
    cmp_result = strcmp(tag_meta->name, tag_name);
    ASSERT(cmp_result == 0);

    Tag_File_Entry *tag_file = get_tag_file_array(mapped_img, fs_meta);
    ASSERT(tag_file->tag_id == 1);
    ASSERT(tag_file->file_id == 1);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src, src_size);
    close(src_fd);

    return true;
}

bool tfsfmt_test_write_files_two_files_one_tag() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);

    char *src_path_1 = "tests/many_sectors.txt";
    char *dst_file_1 = "many_sectors.txt";
    char *file_spec_1 = "tests/many_sectors.txt:many_sectors.txt";
    int src_fd_1 = open(src_path_1, O_RDONLY);
    size_t src_size_1 = get_file_size(src_fd_1);
    uint8_t *mapped_src_1 = mmap(NULL, src_size_1, PROT_READ, MAP_SHARED, src_fd_1, 0);

    char *src_path_2 = "tests/part_sector.txt";
    char *dst_file_2 = "part_sector.txt";
    char *file_spec_2 = "tests/part_sector.txt:part_sector.txt";
    int src_fd_2 = open(src_path_2, O_RDONLY);
    size_t src_size_2 = get_file_size(src_fd_2);
    uint8_t *mapped_src_2 = mmap(NULL, src_size_2, PROT_READ, MAP_SHARED, src_fd_2, 0);

    char *tag_name = "test_tag";

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {
        "./build/tfsfmt", "write-files",
        "-file", file_spec_1,
        "-file", file_spec_2,
        "-tag", tag_name,
        image_name, 0
    };
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    // Check data for many_sectors.txt
    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta[0].first_data_sector == 1);
    ASSERT(file_meta[0].size == src_size_1);
    int cmp_result = strcmp(file_meta[0].name, dst_file_1);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 1);
    ASSERT(fat[1] == 2);
    ASSERT(fat[2] == 0xffff);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src_1, src_size_1);
    ASSERT(cmp_result == 0);

    // Check data for part_sector.txt
    ASSERT(file_meta[1].first_data_sector == 4);
    ASSERT(file_meta[1].size == src_size_2);
    cmp_result = strcmp(file_meta[1].name, dst_file_2);
    ASSERT(cmp_result == 0);

    ASSERT(fat[3] == 0xffff);
    ASSERT(fat[4] == 0);

    cmp_result = memcmp(data + fs_meta->sector_size * 3, mapped_src_2, src_size_2);
    ASSERT(cmp_result == 0);

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta->first_data_sector == 1); // @TODO: May not need this.
    cmp_result = strcmp(tag_meta->name, tag_name);
    ASSERT(cmp_result == 0);

    Tag_File_Entry *tag_file = get_tag_file_array(mapped_img, fs_meta);
    ASSERT(tag_file[0].tag_id == 1);
    ASSERT(tag_file[0].file_id == 1);
    ASSERT(tag_file[1].tag_id == 1);
    ASSERT(tag_file[1].file_id == 2);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src_1, src_size_1);
    close(src_fd_1);
    munmap(mapped_src_2, src_size_2);
    close(src_fd_2);

    return true;
}

bool tfsfmt_test_write_files_one_file_two_tags() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);

    char *src_path = "tests/part_sector.txt";
    char *dst_file = "part_sector.txt";
    char *file_spec = "tests/part_sector.txt:part_sector.txt";
    int src_fd = open(src_path, O_RDONLY);
    size_t src_size = get_file_size(src_fd);
    uint8_t *mapped_src = mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0);

    char *tag_name_1 = "test_tag_1";
    char *tag_name_2 = "test_tag_2";

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {
        "./build/tfsfmt", "write-files",
        "-file", file_spec,
        "-tag", tag_name_1,
        "-tag", tag_name_2,
        image_name, 0
    };
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta->first_data_sector == 1);
    ASSERT(file_meta->size == src_size);
    int cmp_result = strcmp(file_meta->name, dst_file);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 0xffff);
    ASSERT(fat[1] == 0);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src, src_size);
    ASSERT(cmp_result == 0);

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta[0].first_data_sector == 1); // @TODO: May not need this.
    cmp_result = strcmp(tag_meta[0].name, tag_name_1);
    ASSERT(cmp_result == 0);
    ASSERT(tag_meta[1].first_data_sector == 1); // @TODO: May not need this.
    cmp_result = strcmp(tag_meta[1].name, tag_name_2);
    ASSERT(cmp_result == 0);

    Tag_File_Entry *tag_file = get_tag_file_array(mapped_img, fs_meta);
    ASSERT(tag_file[0].tag_id == 1);
    ASSERT(tag_file[0].file_id == 1);
    ASSERT(tag_file[1].tag_id == 2);
    ASSERT(tag_file[1].file_id == 1);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src, src_size);
    close(src_fd);

    return true;
}

bool tfsfmt_test_write_files_two_files_two_tags() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);

    char *src_path_1 = "tests/many_sectors.txt";
    char *dst_file_1 = "many_sectors.txt";
    char *file_spec_1 = "tests/many_sectors.txt:many_sectors.txt";
    int src_fd_1 = open(src_path_1, O_RDONLY);
    size_t src_size_1 = get_file_size(src_fd_1);
    uint8_t *mapped_src_1 = mmap(NULL, src_size_1, PROT_READ, MAP_SHARED, src_fd_1, 0);

    char *src_path_2 = "tests/part_sector.txt";
    char *dst_file_2 = "part_sector.txt";
    char *file_spec_2 = "tests/part_sector.txt:part_sector.txt";
    int src_fd_2 = open(src_path_2, O_RDONLY);
    size_t src_size_2 = get_file_size(src_fd_2);
    uint8_t *mapped_src_2 = mmap(NULL, src_size_2, PROT_READ, MAP_SHARED, src_fd_2, 0);

    char *tag_name_1 = "test_tag_1";
    char *tag_name_2 = "test_tag_2";

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_file_argv[] = {
        "./build/tfsfmt", "write-files",
        "-file", file_spec_1,
        "-file", file_spec_2,
        "-tag", tag_name_1,
        "-tag", tag_name_2,
        image_name, 0
    };
    run_until_completion("./build/tfsfmt", write_file_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    // Check data for many_sectors.txt
    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    ASSERT(file_meta[0].first_data_sector == 1);
    ASSERT(file_meta[0].size == src_size_1);
    int cmp_result = strcmp(file_meta[0].name, dst_file_1);
    ASSERT(cmp_result == 0);

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    ASSERT(fat[0] == 1);
    ASSERT(fat[1] == 2);
    ASSERT(fat[2] == 0xffff);

    uint8_t *data = get_data(mapped_img, fs_meta);
    cmp_result = memcmp(data, mapped_src_1, src_size_1);
    ASSERT(cmp_result == 0);

    // Check data for part_sector.txt
    ASSERT(file_meta[1].first_data_sector == 4);
    ASSERT(file_meta[1].size == src_size_2);
    cmp_result = strcmp(file_meta[1].name, dst_file_2);
    ASSERT(cmp_result == 0);

    ASSERT(fat[3] == 0xffff);
    ASSERT(fat[4] == 0);

    cmp_result = memcmp(data + fs_meta->sector_size * 3, mapped_src_2, src_size_2);
    ASSERT(cmp_result == 0);

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta[0].first_data_sector == 1); // @TODO: May not need this.
    cmp_result = strcmp(tag_meta[0].name, tag_name_1);
    ASSERT(cmp_result == 0);
    ASSERT(tag_meta[1].first_data_sector == 1); // @TODO: May not need this.
    cmp_result = strcmp(tag_meta[1].name, tag_name_2);
    ASSERT(cmp_result == 0);

    Tag_File_Entry *tag_file = get_tag_file_array(mapped_img, fs_meta);
    ASSERT(tag_file[0].tag_id == 1);
    ASSERT(tag_file[0].file_id == 1);
    ASSERT(tag_file[1].tag_id == 2);
    ASSERT(tag_file[1].file_id == 1);
    ASSERT(tag_file[2].tag_id == 1);
    ASSERT(tag_file[2].file_id == 2);
    ASSERT(tag_file[3].tag_id == 2);
    ASSERT(tag_file[3].file_id == 2);

    munmap(mapped_img, image_size);
    close(fd);
    munmap(mapped_src_1, src_size_1);
    close(src_fd_1);
    munmap(mapped_src_2, src_size_2);
    close(src_fd_2);

    return true;
}



// --------------------------------------------------
// WRITE-TAG SUBCOMMAND TESTS
// --------------------------------------------------

bool tfsfmt_test_write_tag() {
    size_t image_size = 512 * 1000;
    int fd = create_fs_image(image_name, image_size);
    char *tag_name = "mytag";

    char *format_argv[] = {"./build/tfsfmt", "format", image_name, 0};
    run_until_completion("./build/tfsfmt", format_argv);

    char *write_tag_argv[] = {"./build/tfsfmt", "write-tag", "-tag", tag_name, image_name, 0};
    run_until_completion("./build/tfsfmt", write_tag_argv);

    uint8_t *mapped_img = mmap(NULL, image_size, PROT_READ, MAP_SHARED, fd, 0);

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    ASSERT(tag_meta->first_data_sector == 1);
    int cmp_result = strcmp(tag_meta->name, tag_name);
    ASSERT(cmp_result == 0);

    munmap(mapped_img, image_size);
    close(fd);

    return true;
}



// --------------------------------------------------
// TEST ARRAY
// --------------------------------------------------

Test tests[] = {
    TEST(tfsfmt_test_format_with_default_args),
    TEST(tfsfmt_test_format_with_custom_args),
    TEST(tfsfmt_test_write_files_less_than_one_sector),
    TEST(tfsfmt_test_write_files_exactly_one_sector),
    TEST(tfsfmt_test_write_files_many_sectors),
    TEST(tfsfmt_test_write_files_two_files),
    TEST(tfsfmt_test_write_files_same_file),
    TEST(tfsfmt_test_write_files_one_file_one_tag),
    TEST(tfsfmt_test_write_files_two_files_one_tag),
    TEST(tfsfmt_test_write_files_one_file_two_tags),
    TEST(tfsfmt_test_write_files_two_files_two_tags),
    //TEST(tfsfmt_test_write_tag),
};
