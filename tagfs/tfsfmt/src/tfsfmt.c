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
#include "utils.c"

void print_usage(char *program_name) {
    printf("Usage: %s <subcommand> [OPTIONS]\n\n", program_name);
    printf("Subcommands:\n");
    printf("  format: format a file system image.\n");
}



// --------------------------------------------------
// FORMAT SUBCOMMAND
// --------------------------------------------------

typedef struct {
    int sector_count;
    int sector_size;

    int file_meta_sector_count;
    int tag_meta_sector_count;
    int tag_file_sector_count;
    int fat_sector_count;

    char *image_name;
} Format_Options;

void parse_format_options(Format_Options *options, int argc, char **argv) {
    // Set up defaults.
    options->sector_count = 1000;
    options->sector_size = 512;
    options->file_meta_sector_count = 10;
    options->tag_meta_sector_count = 10;
    options->tag_file_sector_count = 10;
    options->fat_sector_count = 2;
    options->image_name = NULL;

    // Parse the given options.
    flags_add_cstr_positional(&options->image_name, "image_name", "name of the image to format");
    flags_add_int_flag(&options->sector_count, "-sector-count", "number of sectors in the image", false);
    flags_add_int_flag(&options->sector_size, "-sector-size", "size of a sector", false);
    flags_add_int_flag(&options->file_meta_sector_count, "-fm-sectors", "number of sectors in the file metadata section", false);
    flags_add_int_flag(&options->tag_meta_sector_count, "-tm-sectors", "number of sectors in the tag metadata section", false);
    flags_add_int_flag(&options->tag_file_sector_count, "-tf-sectors", "number of sectors in the tag file map", false);
    flags_add_int_flag(&options->fat_sector_count, "-fat-sectors", "number of sectors in the file allocation table (FAT)", false);

    char *program_name = shift_args(&argc, &argv);
    char *subcommand = shift_args(&argc, &argv);
    if (!flags_parse_flags(argc, argv)) {
        // @TODO: Temporary memory arena allocator.
        // @TODO: Add subcommand support to flag library?
        char *prefix = calloc(strlen(program_name) + strlen(subcommand) + 2, 1);
        sprintf(prefix, "%s %s", program_name, subcommand);
        flags_print_help(prefix);
        exit(1);
    }
}

void format_image(int argc, char **argv) {
    // Parse command line options.
    Format_Options options;
    parse_format_options(&options, argc, argv);

    int fd = open(options.image_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }

    uint8_t *mapped_img = mmap(NULL, options.sector_count * options.sector_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_img == MAP_FAILED) {
        fprintf(stderr, "ERROR: Could not mmap file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }

    // Write the FS metadata block.
    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    fs_meta->version = 1;
    fs_meta->sector_count = options.sector_count;
    fs_meta->sector_size = options.sector_size;
    fs_meta->file_meta_sector_count = options.file_meta_sector_count;
    fs_meta->tag_meta_sector_count = options.tag_meta_sector_count;
    fs_meta->tag_file_sector_count = options.tag_file_sector_count;
    fs_meta->fat_sector_count = options.fat_sector_count;
    fs_meta->free_file_id = 1;
    fs_meta->free_tag_id = 1;

    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    file_meta->id = 0;

    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    tag_meta->id = 0;

    Tag_File_Entry *tag_file = get_tag_file_array(mapped_img, fs_meta);
    tag_file->tag_id = 0;

    uint16_t *fat = get_fat(mapped_img, fs_meta);
    size_t fat_byte_count = fs_meta->fat_sector_count * fs_meta->sector_size;
    memset(fat, 0, fat_byte_count);

    // Make sure the changes propagate to the file.
    if (msync(mapped_img, options.sector_count * options.sector_size, MS_SYNC) == -1) {
        fprintf(stderr, "ERROR: Could not sync changes to file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }
}



// --------------------------------------------------
// WRITE-FILE SUBCOMMAND
// --------------------------------------------------

typedef struct {
    char *src_file_path;
    char *dst_file_name;

    char *image_name;
} Write_File_Options;

void parse_write_file_options(Write_File_Options *options, int argc, char **argv) {
    // Set default dst_file_name.
    options->dst_file_name = NULL;

    flags_add_cstr_positional(&options->image_name, "image_name", "name of the image to format");
    flags_add_cstr_flag(&options->src_file_path, "-src", "path of the file to copy to the image", true);
    flags_add_cstr_flag(&options->dst_file_name, "-dst", "name of the file on the image", false);

    char *program_name = shift_args(&argc, &argv);
    char *subcommand = shift_args(&argc, &argv);
    if (!flags_parse_flags(argc, argv)) {
        // @TODO: Temporary memory arena allocator.
        // @TODO: Add subcommand support to flag library?
        char *prefix = calloc(strlen(program_name) + strlen(subcommand) + 2, 1);
        sprintf(prefix, "%s %s", program_name, subcommand);
        flags_print_help(prefix);
        free(prefix);
        exit(1);
    }

    // Fill in dst_file_name if not provided.
    if (options->dst_file_name == NULL) {
        char *last_slash = strrchr(options->src_file_path, '/');
        if (last_slash == NULL) {
            options->dst_file_name = options->src_file_path;
        }
        else {
            options->dst_file_name = last_slash + 1;
        }
    }
}

void write_file(int argc, char **argv) {
    Write_File_Options options = {0};
    parse_write_file_options(&options, argc, argv);

    int fd = open(options.image_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }

    FS_Metadata fs_meta;
    read_fs_metadata(&fs_meta, fd);

    uint8_t *mapped_img = mmap(NULL, fs_meta.sector_count * fs_meta.sector_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_img == MAP_FAILED) {
        fprintf(stderr, "ERROR: Could not mmap file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }

    File_Metadata *file_meta = get_file_metadata(mapped_img, &fs_meta);

    // Find the first free file metadata entry.
    while (file_meta->id != 0) {
        file_meta++;
    }

    // Update the id.
    file_meta->id = fs_meta.free_file_id;
    ((FS_Metadata*)mapped_img)->free_file_id++;
    (file_meta + 1)->id = 0;

    // Copy the data over.
    FILE *src_file = fopen(options.src_file_path, "rb");
    if (src_file == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s\n.", options.src_file_path);
        exit(1);
    }
    uint16_t *fat = get_fat(mapped_img, &fs_meta);
    uint8_t *data = get_data(mapped_img, &fs_meta);
    int num_fat_entries = fs_meta.fat_sector_count * fs_meta.sector_size / 2;

    int fat_index = 0;
    for (;fat_index < num_fat_entries; fat_index++) {
        if (fat[fat_index] == 0) {
            break;
        }
    }
    if (fat_index == num_fat_entries) {
        fprintf(stderr, "WARNING: File has been truncated. Not enough space.\n");
        return;
    }

    file_meta->first_data_sector = fat_index;

    for (;;) {
        fread(data + fat_index * fs_meta.sector_size, fs_meta.sector_size, 1, src_file);
        if (feof(src_file)) {
            fat[fat_index] = 0xffff;
            break;
        }

        int old_fat_index = fat_index;
        fat_index++;
        for (;fat_index < num_fat_entries; fat_index++) {
            if (fat[fat_index] == 0) {
                break;
            }
        }
        if (fat_index == num_fat_entries) {
            fprintf(stderr, "WARNING: File has been truncated. Not enough space.\n");
            break;
        }
        fat[old_fat_index] = fat_index;
    }

    // Set the name.
    if (strlen(options.dst_file_name) > 25) {
        fprintf(stderr, "ERROR: Destination file name too long. Must be less than 26 characters.");
        exit(1);
    }
    strcpy(file_meta->name, options.dst_file_name);

    // Make sure the changes propagate to the file.
    if (msync(mapped_img, fs_meta.sector_count * fs_meta.sector_size, MS_SYNC) == -1) {
        fprintf(stderr, "ERROR: Could not sync changes to file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }
}



int main(int argc, char **argv) {
    char *subcommand = argv[1];
    if (subcommand == NULL) {
        printf("No subcommand provided.\n\n");
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(subcommand, "format") == 0) {
        format_image(argc, argv);
    }
    else if (strcmp(subcommand, "write-file") == 0) {
        write_file(argc, argv);
    }
    else {
        printf("Unknown subcommand %s\n\n.", subcommand);
        print_usage(argv[0]);
        return 1;
    }
}
