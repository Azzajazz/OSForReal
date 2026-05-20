#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../../flags.c"

typedef struct {
    uint8_t magic[4]; // Contains "DRIV"
    uint8_t checksum; // All bytes in the header and driver entries must add to 0 mod 0x100.
    uint8_t version;
    uint16_t driver_count;
} Driver_Header;

typedef struct {
    uint8_t vendor_id;
    uint8_t device_id;
    uint16_t sector_start; // Sector offset from the start of the header.
    uint16_t sector_count;
} Driver_Entry;



#define MAX_DRIVERS 25
#define BOOT_PARTITION_SECTORS 512
#define SECTOR_SIZE 512

typedef struct {
    char *image_name;
    char *driver_specs[MAX_DRIVERS];
    int driver_specs_parsed;
} Options;
Options options = {0};

typedef struct {
    char *file_path;
    bool parsed;
    uint8_t vendor_id;
    uint8_t device_id;
} Driver_Spec;

Driver_Spec parse_driver_spec(char *driver_spec) {
    Driver_Spec spec = {0};
    char *vendor_id_str = strtok(driver_spec, ":");
    if (vendor_id_str == NULL) {
        return spec;
    }
    char *device_id_str = strtok(NULL, ":");
    if (device_id_str == NULL) {
        return spec;
    }
    char *file_path = strtok(NULL, "\0");
    if (file_path == NULL) {
        return spec;
    }

    spec.file_path = file_path;
    spec.parsed = true;
    spec.vendor_id = atoi(vendor_id_str);
    spec.device_id = atoi(device_id_str);
    return spec;
}

bool write_driver(char *driver_spec, uint8_t *mapped_img, Driver_Entry *entry, uint16_t sector_offset) {
    Driver_Spec spec = parse_driver_spec(driver_spec); 
    if (!spec.parsed) {
        fprintf(stderr, "SKIPPING %s: invalid driver specification.", driver_spec);
        return false;
    }

    FILE *f = fopen(spec.file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "SKIPPING %s: file not found.", driver_spec);
        return false;
    }

    size_t bytes_copied = 0;
    while (!feof(f)) {
        bytes_copied += fread(mapped_img + sector_offset * SECTOR_SIZE + bytes_copied, 1, SECTOR_SIZE, f);
    }

    entry->vendor_id = spec.vendor_id;
    entry->device_id = spec.device_id;
    entry->sector_start = sector_offset;
    entry->sector_count = (bytes_copied + SECTOR_SIZE - 1) / SECTOR_SIZE;
    return true;
}

void write_drivers(uint8_t *mapped_img) {
    uint16_t metadata_bytes = sizeof(Driver_Header) + sizeof(Driver_Entry) * options.driver_specs_parsed;
    uint16_t data_sector_offset = (metadata_bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;

    Driver_Entry *entries = (Driver_Entry *)(mapped_img + sizeof(Driver_Header));
    for (int i = 0; i < options.driver_specs_parsed; i++) {
        bool success = write_driver(options.driver_specs[i], mapped_img, &entries[i], data_sector_offset);
        if (success) {
            data_sector_offset += entries[i].sector_count;
        }
    }
}

uint8_t calculate_checksum(uint8_t *mapped_img, Driver_Header *header) {
    uint8_t sum = 0;

    uint8_t *header_bytes = (uint8_t *)header;
    for (size_t i = 0; i < sizeof(*header); i++) {
        sum += header_bytes[i];
    }
    size_t remaining_bytes = options.driver_specs_parsed * sizeof(Driver_Entry);
    uint8_t *entries_start = mapped_img + sizeof(*header);
    for (size_t i = 0; i < remaining_bytes; i++) {
        sum += entries_start[i];
    }

    return -sum;
}

int main(int argc, char **argv) {
    char *program_name = shift_args(&argc, &argv);
    flags_add_cstr_positional(&options.image_name, "image_name", "name of the image to write to.");
    flags_add_cstr_array_flag(options.driver_specs, &options.driver_specs_parsed, MAX_DRIVERS, "-driver", "specification of a driver to copy. Driver specifications are of the form <vendor_id>:<device_id>:<path_to_shared_object>.", true);
    if (!flags_parse_flags(argc, argv)) {
        flags_print_help(program_name);
        exit(1);
    }

    int fd = open(options.image_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }
    uint8_t *mapped_img = mmap(NULL, (BOOT_PARTITION_SECTORS + 1) * SECTOR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_img == MAP_FAILED) {
        fprintf(stderr, "ERROR: Could not mmap file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }

    write_drivers(mapped_img + SECTOR_SIZE);
    Driver_Header header = {
        .magic = {'D', 'R', 'I', 'V'},
        .checksum = 0, // Filled in later.
        .version = 1,
        .driver_count = options.driver_specs_parsed,
    };
    header.checksum = calculate_checksum(mapped_img + SECTOR_SIZE, &header);
    memcpy(mapped_img + SECTOR_SIZE, &header, sizeof(header));


    if (msync(mapped_img, BOOT_PARTITION_SECTORS * SECTOR_SIZE, MS_SYNC) == -1) {
        fprintf(stderr, "ERROR: Could not sync changes to file %s: %s\n", options.image_name, strerror(errno));
        exit(1);
    }
}
