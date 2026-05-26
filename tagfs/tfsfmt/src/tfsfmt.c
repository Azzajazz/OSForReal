#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "flags.c"
#include "../../utils.c"


void print_usage(char *program_name) {
    printf("Usage: %s <subcommand> [OPTIONS]\n\n", program_name);
    printf("Subcommands:\n");
    printf("  format: format a file system image.\n");
    // @TODO: Add subcommand support to flags library.
}

size_t get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    return size;
}

int filter_valid_tag_names(
    char **tag_names, int tag_names_count,
    char **filtered_tag_names
) {
    int filtered_tag_names_count = 0;

    for (int i = 0; i < tag_names_count; i++) {
        if (strlen(tag_names[i]) < 28) {
            filtered_tag_names[filtered_tag_names_count] = tag_names[i];
            filtered_tag_names_count++;
        }
        else {
            fprintf(stderr, "SKIPPING %s: Tag name too long. Must be less than 28 characters.\n", tag_names[i]);
        }
    }

    return filtered_tag_names_count;
}

uint16_t find_or_create_tag(FS_Metadata *fs_meta, Tag_Metadata *tag_meta, char *name) {
    unsigned int tag_meta_entry_count = fs_meta->tag_meta_sector_count * fs_meta->sector_size / sizeof(Tag_Metadata);

    // Try to find the tag.
    for (uint16_t index = 0; index < tag_meta_entry_count; index++) {
        if (strcmp(tag_meta[index].name, name) == 0) {
            return index + 1;
        }
    }

    // Otherwise, create the tag and return the index.
    for (uint16_t index = 0; index < tag_meta_entry_count; index++) {
        if (tag_metadata_entry_is_free(&tag_meta[index])) {
            strcpy(tag_meta[index].name, name);
            return index + 1;
        }
    }

    return 0;
}

bool link_tag_to_file(
    FS_Metadata *fs_meta,
    Tag_File_Entry *tag_file,
    uint16_t tag_id, uint16_t file_id
) {
    int tag_file_entry_count = fs_meta->tag_file_sector_count * fs_meta->sector_size / sizeof(Tag_File_Entry);

    // If the file and tag are already linked, then nothing to do.
    for (int i = 0; i < tag_file_entry_count; i++) {
        if (tag_file[i].tag_id == tag_id && tag_file[i].file_id == file_id) {
            return true;
        }
    }

    // Otherwise, create the link.
    for (int i = 0; i < tag_file_entry_count; i++) {
        if (tag_file[i].tag_id == 0) {
            tag_file[i].tag_id = tag_id;
            tag_file[i].file_id = file_id;
            return true;
        }
    }

    return false;
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
        exit(FAIL_PARSE_ARGS);
    }
}

void format_image(int argc, char **argv) {
    // Parse command line options.
    Format_Options options;
    parse_format_options(&options, argc, argv);

    int fd = open(options.image_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_IMG_NOT_FOUND);
    }

    uint8_t *mapped_img = mmap(NULL, options.sector_count * options.sector_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_img == MAP_FAILED) {
        fprintf(stderr, "ERROR: Could not mmap file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_GENERIC);
    }

    memset(mapped_img, 0, options.sector_count * options.sector_size);

    // Write the FS metadata block.
    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    fs_meta->version = 1;
    fs_meta->sector_count = options.sector_count;
    fs_meta->sector_size = options.sector_size;
    fs_meta->file_meta_sector_count = options.file_meta_sector_count;
    fs_meta->tag_meta_sector_count = options.tag_meta_sector_count;
    fs_meta->tag_file_sector_count = options.tag_file_sector_count;
    fs_meta->fat_sector_count = options.fat_sector_count;

    // Make sure the changes propagate to the file.
    if (msync(mapped_img, options.sector_count * options.sector_size, MS_SYNC) == -1) {
        fprintf(stderr, "ERROR: Could not sync changes to file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_GENERIC);
    }
}



// --------------------------------------------------
// WRITE-FILES SUBCOMMAND
// --------------------------------------------------

typedef struct {
    char *file_specs[32];
    int file_specs_parsed;
    char *tags[32];
    int tags_parsed;

    char *image_name;
} Write_Files_Options;

void parse_write_files_options(Write_Files_Options *options, int argc, char **argv) {
    flags_add_cstr_positional(&options->image_name, "image_name", "name of the image to format");
    flags_add_cstr_array_flag(options->file_specs, &options->file_specs_parsed, ARRAY_LEN(options->file_specs), "-file", "source path and destination name of the file in `<source path>:<destination name>` format", true);
    // @TODO: This is a lot of arguments now. Not sure I like it.
    // Maybe this is a place for some sort of bounded dynamic array type?
    flags_add_cstr_array_flag(options->tags, &options->tags_parsed, ARRAY_LEN(options->tags), "-tag", "name of a tag to link to provided files", false);

    char *program_name = shift_args(&argc, &argv);
    char *subcommand = shift_args(&argc, &argv);
    if (!flags_parse_flags(argc, argv)) {
        // @TODO: Temporary memory arena allocator.
        // @TODO: Add subcommand support to flag library?
        char *prefix = calloc(strlen(program_name) + strlen(subcommand) + 2, 1);
        sprintf(prefix, "%s %s", program_name, subcommand);
        flags_print_help(prefix);
        free(prefix);
        exit(FAIL_PARSE_ARGS);
    }
}

bool is_tagfs_file(char *src_file_path, char *dst_file_name, size_t file_size) {
    if (file_size > UINT32_MAX) {
        fprintf(stderr, "SKIPPING %s: File is too big. Maximum size is %"PRIu32" bytes.\n", src_file_path, UINT32_MAX);
        return false;
    }

    if (strlen(dst_file_name) >= 24) {
        fprintf(stderr, "SKIPPING %s: Destination file name too long. Must be less than 24 characters.\n", src_file_path);
        return false;
    }

    return true;
}

void write_files(int argc, char **argv) {
    Write_Files_Options options = {0};
    parse_write_files_options(&options, argc, argv);

    char *tags[ARRAY_LEN(options.tags)] = {0};
    int tags_count = filter_valid_tag_names(options.tags, options.tags_parsed, tags);

    int fd = open(options.image_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_IMG_NOT_FOUND);
    }

    FS_Metadata fs_meta_for_size;
    read_fs_metadata(&fs_meta_for_size, fd);

    uint8_t *mapped_img = mmap(NULL, fs_meta_for_size.sector_count * fs_meta_for_size.sector_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_img == MAP_FAILED) {
        fprintf(stderr, "ERROR: Could not mmap file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_GENERIC);
    }

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, fs_meta);
    Tag_File_Entry *tag_file = get_tag_file_array(mapped_img, fs_meta);
    uint16_t *fat = get_fat(mapped_img, fs_meta);
    uint8_t *data = get_data(mapped_img, fs_meta);

    for (int i = 0; i < options.file_specs_parsed; ++i) {
        char *src_file_path = strtok(options.file_specs[i], ":");
        char *dst_file_name = strtok(NULL, "\0");
        if (dst_file_name == NULL) {
            // This was not a colon-separated file spec. Set the destination name based
            // on the source file path.
            char *last_slash = strrchr(src_file_path, '/');
            if (last_slash == NULL) {
                dst_file_name = src_file_path;
            }
            else {
                dst_file_name = last_slash + 1;
            }
        }

        FILE *src_file = fopen(src_file_path, "rb");
        if (src_file == NULL) {
            fprintf(stderr, "SKIPPING %s: File not found.\n", src_file_path);
            continue;
        }
        size_t src_size = get_file_size(src_file);
        if (!is_tagfs_file(src_file_path, dst_file_name, src_size)) {
            goto fail;
        }

        unsigned int file_meta_count = fs_meta->file_meta_sector_count * fs_meta->sector_size / sizeof(File_Metadata);
        uint16_t file_id = 0;
        for (; file_id < file_meta_count; file_id++) {
            if (file_metadata_entry_is_free(&file_meta[file_id])) {
                break;
            }
        }
        if (file_id == file_meta_count) {
            fprintf(stderr, "SKIPPING %s: File metadata is full.\n", src_file_path);
            continue;
        }
        file_id++;

        // Copy the data over.
        int num_fat_entries = fs_meta->fat_sector_count * fs_meta->sector_size / 2;

        int fat_index = 0;
        for (;fat_index < num_fat_entries; fat_index++) {
            if (fat[fat_index] == 0) {
                break;
            }
        }
        if (fat_index == num_fat_entries) {
            fprintf(stderr, "WARNING: File %s has been truncated. Not enough space.\n", dst_file_name);
            continue;
        }

        file_meta[file_id - 1].first_data_sector = fat_index;

        size_t bytes_copied = 0;
        for (;;) {
            bytes_copied += fread(data + fat_index * fs_meta->sector_size, 1, fs_meta->sector_size, src_file);
            if (bytes_copied == src_size) {
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
                fprintf(stderr, "WARNING: File %s has been truncated. Not enough space.\n", dst_file_name);
                fat[old_fat_index] = 0xffff;
                break;
            }
            fat[old_fat_index] = fat_index;
        }

        // Update the size.
        file_meta[file_id - 1].size = bytes_copied;

        // Set the name.
        strcpy(file_meta[file_id - 1].name, dst_file_name);

        // Link the tags.
        for (int i = 0; i < tags_count; i++) {
            uint16_t tag_id = find_or_create_tag(fs_meta, tag_meta, tags[i]);
            if (tag_id > 0) {
                if (!link_tag_to_file(fs_meta, tag_file, tag_id, file_id)) {
                    fprintf(stderr, "WARNING: Could not link tag %s to file %s; tag file map is full.\n", tags[i], dst_file_name);
                }
            }
            else {
                fprintf(stderr, "WARNING: Could not link tag %s to file %s; the tag needed to be created, but the tag metadata section is full.\n", tags[i], dst_file_name);
            }
        }

fail:
        fclose(src_file);
    }

    // Make sure the changes propagate to the file.
    if (msync(mapped_img, fs_meta->sector_count * fs_meta->sector_size, MS_SYNC) == -1) {
        fprintf(stderr, "ERROR: Could not sync changes to file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_GENERIC);
    }
}



// --------------------------------------------------
// WRITE-TAG SUBCOMMAND
// --------------------------------------------------

typedef struct {
    char *image_name;
    char *tag_name;
} Write_Tag_Options;

void parse_write_tag_options(Write_Tag_Options *options, int argc, char **argv) {
    flags_add_cstr_positional(&options->image_name, "image_name", "name of the image to format");
    flags_add_cstr_flag(&options->tag_name, "-tag", "name of the tag to write", true);

    char *program_name = shift_args(&argc, &argv);
    char *subcommand = shift_args(&argc, &argv);
    if (!flags_parse_flags(argc, argv)) {
        // @TODO: Temporary memory arena allocator.
        // @TODO: Add subcommand support to flag library?
        char *prefix = calloc(strlen(program_name) + strlen(subcommand) + 2, 1);
        sprintf(prefix, "%s %s", program_name, subcommand);
        flags_print_help(prefix);
        free(prefix);
        exit(FAIL_PARSE_ARGS);
    }
}

void write_tag(int argc, char **argv) {
    Write_Tag_Options options;
    parse_write_tag_options(&options, argc, argv);

    int fd = open(options.image_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_IMG_NOT_FOUND);
    }

    FS_Metadata fs_meta;
    read_fs_metadata(&fs_meta, fd);

    uint8_t *mapped_img = mmap(NULL, fs_meta.sector_count * fs_meta.sector_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_img == MAP_FAILED) {
        fprintf(stderr, "ERROR: Could not mmap file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_GENERIC);
    }

    // Find the first empty tag metadata entry
    Tag_Metadata *tag_meta = get_tag_metadata(mapped_img, &fs_meta);
    while (tag_metadata_entry_is_free(tag_meta)) {
        tag_meta++;
    }

    // Set the name.
    if (strlen(options.tag_name) >= 28) {
        fprintf(stderr, "ERROR: Destination file name too long. Must be less than 28 characters.");
        exit(FAIL_GENERIC);
    }
    strcpy(tag_meta->name, options.tag_name);

    // Make sure the changes propagate to the file.
    if (msync(mapped_img, fs_meta.sector_count * fs_meta.sector_size, MS_SYNC) == -1) {
        fprintf(stderr, "ERROR: Could not sync changes to file %s: %s\n", options.image_name, strerror(errno));
        exit(FAIL_GENERIC);
    }
}



int main(int argc, char **argv) {
    char *subcommand = argv[1];
    if (subcommand == NULL) {
        printf("No subcommand provided.\n\n");
        print_usage(argv[0]);
        exit(FAIL_PARSE_ARGS);
    }

    if (strcmp(subcommand, "format") == 0) {
        format_image(argc, argv);
    }
    else if (strcmp(subcommand, "write-files") == 0) {
        write_files(argc, argv);
    }
    else if (strcmp(subcommand, "write-tag") == 0) {
        write_tag(argc, argv);
    }
    else {
        printf("Unknown subcommand %s\n\n.", subcommand);
        print_usage(argv[0]);
        exit(FAIL_PARSE_ARGS);
    }
}
