#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../flags.c"

typedef enum {
    PT_BOOT,
    PT_GENERIC,
} Partition_Type;

typedef struct {
    uint8_t magic[4]; // Contains "PART"
    uint8_t checksum; // All bytes must add to 0 mod 0x100 (including header and all partition entries).
    uint8_t version;
    uint16_t partition_count;
} Partition_Header;

typedef struct {
    Partition_Type type;
    uint32_t sector_start;
    uint32_t sector_count;
} Partition_Entry;


uint8_t calculate_checksum(Partition_Header header, Partition_Entry boot, Partition_Entry generic) {
    uint8_t *header_bytes = (uint8_t *)&header;
    uint8_t *boot_bytes = (uint8_t *)&boot;
    uint8_t *generic_bytes = (uint8_t *)&generic;

    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(header); i++) {
        sum += header_bytes[i];
    }
    for (size_t i = 0; i < sizeof(boot); i++) {
        sum += boot_bytes[i];
    }
    for (size_t i = 0; i < sizeof(generic); i++) {
        sum += generic_bytes[i];
    }

    return -sum;
}

#define BOOT_PARTITION_SIZE 262144
#define HARD_DISK_SIZE 1048576

int main(int argc, char **argv) {
    char *program_name = shift_args(&argc, &argv);

    char *image_path = NULL;
    flags_add_cstr_positional(&image_path, "image_path", "path of the image to partition.");
    if (!flags_parse_flags(argc, argv)) {
        flags_print_help(program_name);
        exit(1);
    }

    FILE *f = fopen(image_path, "rb+");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open image %s\n", image_path);
        exit(1);
    }

    //
    // This is basically a script, so everything here is hard-coded.
    //
    Partition_Header header = {
        .magic = {'P', 'A', 'R', 'T'},
        .checksum = 0, // To be filled in later.
        .version = 1,
        .partition_count = 2,
    };

    Partition_Entry boot = {
        .type = PT_BOOT,
        .sector_start = 1,
        .sector_count = BOOT_PARTITION_SIZE / 512,
    };

    Partition_Entry generic = {
        .type = PT_GENERIC,
        .sector_start = boot.sector_start + boot.sector_count,
        .sector_count = (HARD_DISK_SIZE - BOOT_PARTITION_SIZE - 1) / 512,
    };

    header.checksum = calculate_checksum(header, boot, generic);

    fwrite(&header, sizeof(header), 1, f);
    fwrite(&boot, sizeof(boot), 1, f);
    fwrite(&generic, sizeof(generic), 1, f);

    fclose(f);
    return 0;
}
