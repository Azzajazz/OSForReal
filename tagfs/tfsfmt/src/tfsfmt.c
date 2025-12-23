#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "flags.c"

struct {
    int sector_count;
    int sector_size;

    int file_meta_sector_count;
    int tag_meta_sector_count;
    int tag_file_sector_count;
    int fat_sector_count;
} options;

// @TODO: Customize with cmd-line arguments.
#define SECTOR_COUNT 1000
#define SECTOR_SIZE 512

typedef struct {
    uint32_t version;
    uint32_t sector_count;
    uint32_t sector_size;
    uint32_t file_meta_sector_count;
    uint32_t tag_meta_sector_count;
    uint32_t tag_file_sector_count;
    uint32_t fat_sector_count;
} FS_Metadata;

void parse_options(int argc, char **argv) {
    // Set up defaults.
    options.sector_count = 1000;
    options.sector_size = 512;
    options.file_meta_sector_count = 10;
    options.tag_meta_sector_count = 10;
    options.tag_file_sector_count = 10;
    options.fat_sector_count = 2;

    // Parse the given options.
    flags_add_int_flag(&options.sector_count, "-sector-count");
    flags_add_int_flag(&options.sector_size, "-sector-size");
    flags_add_int_flag(&options.file_meta_sector_count, "-fm-sectors");
    flags_add_int_flag(&options.tag_meta_sector_count, "-tm-sectors");
    flags_add_int_flag(&options.tag_file_sector_count, "-tf-sectors");
    flags_add_int_flag(&options.fat_sector_count, "-fat_sectors");

    shift_args(&argc, &argv);
    flags_parse_flags(argc, argv);
}

int main(int argc, char **argv) {
    // Parse command line options.
    // @TODO: Flag parsing library.
    parse_options(argc, argv);
    char *image_name = "test.img";

    if (image_name == NULL) {
        fprintf(stderr, "ERROR: Must supply an image name\n");
    }

    char *file_name = "test.img";
    int fd = open(file_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", file_name, strerror(errno));
        return 1;
    }

    uint8_t *mapped_img = mmap(NULL, SECTOR_COUNT * SECTOR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_img == MAP_FAILED) {
        fprintf(stderr, "ERROR: Could not mmap file %s: %s\n", file_name, strerror(errno));
        close(fd);
        return 1;
    }

    // Write the FS metadata block.
    FS_Metadata *fs_meta = (FS_Metadata*)(mapped_img + SECTOR_SIZE);
    fs_meta->version = 1;
    fs_meta->sector_count = options.sector_count;
    fs_meta->sector_size = options.sector_size;
    fs_meta->file_meta_sector_count = options.file_meta_sector_count;
    fs_meta->tag_meta_sector_count = options.tag_meta_sector_count;
    fs_meta->tag_file_sector_count = options.tag_file_sector_count;
    fs_meta->fat_sector_count = options.fat_sector_count;

    // Make sure the changes propagate to the file.
    if (msync(mapped_img, SECTOR_COUNT * SECTOR_SIZE, MS_SYNC) == -1) {
        fprintf(stderr, "ERROR: Could not sync changes to file %s: %s\n", file_name, strerror(errno));
        munmap(mapped_img, SECTOR_COUNT * SECTOR_SIZE);
        close(fd);
        return 1;
    }

    munmap(mapped_img, SECTOR_COUNT * SECTOR_SIZE);
    close(fd);
    return 0;
}
