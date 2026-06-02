#include "../common.h"
#include "forward.h"

void assert(char *file, int line, const char *func, bool condition, char *message);
#define ASSERT(condition, message) assert(__FILE__, __LINE__, __func__, (condition), (message))
#define TODO(message) ASSERT(false, "TODO: "message)

// Linker symbols
extern char __boot_start[];
extern char __boot_end[];
extern char __kernel_phys_start[];
extern char __kernel_phys_end[];
extern char __kernel_start[];
extern char __kernel_end[];

#include "../arch/x86/instructions.c"
#include "../arch/x86/interrupts.c"
#include "../hal/hal.c"
#include "../std/std.c"
#include "fs/fs.c"
#include "page_frame_allocator.c"
#include "elf.c"


void assert(char *file, int line, const char *func, bool condition, char *message) {
    if (condition) {
        return;
    }

    fmt_print("ASSERTION FAILED\n");
    fmt_print("%s:%s:%d: %s\n", file, func, line, message);
    cpu_halt();
}

void keyboard_handler(Interrupt_Frame *frame) {
    UNUSED(frame);
    fmt_print("Key press!");
}

void kernel_init(Multiboot_Info *boot_info) {
	terminal_init();
    bool initted = serial_init();
    ASSERT(initted, "Serial initialization failed.");
    serial_write('\n'); // @Hack: Qemu doesn't put a newline in the serial.
    interrupts_init();
    initted = pfa_init(boot_info);
    ASSERT(initted, "Page frame allocation initialization failed.");
    initted = memory_init();
    ASSERT(initted, "Memory initialization failed.");
    initted = pci_init();
    ASSERT(initted, "PCI initialization failed.");
    initted = tfs_init();
    ASSERT(initted, "TagFS initialization failed.");
}

void kernel_main(Multiboot_Info *boot_info) {
    kernel_init(boot_info);

    MMap_Segment *segment = (MMap_Segment *)boot_info->mmap_addr;
    size_t bytes_traversed = 0;
    while (bytes_traversed < boot_info->mmap_length) {
        fmt_print("addr: %lx, length: %lx, type: %u, size: %u\n",
            segment->base_addr, segment->length, segment->type, segment->size);

        bytes_traversed += segment->size + 4;
        segment = (MMap_Segment*)((uint8_t*)segment + segment->size + 4);
    }

    fmt_print("\n");
    fmt_print("boot_start: %x\n", (uint32_t)__boot_start);
    fmt_print("boot_end: %x\n", (uint32_t)__boot_end);
    fmt_print("kernel_phys_start: %x\n", (uint32_t)__kernel_phys_start);
    fmt_print("kernel_phys_end: %x\n", (uint32_t)__kernel_phys_end);
    fmt_print("kernel_start: %x\n", (uint32_t)__kernel_start);
    fmt_print("kernel_end: %x\n", (uint32_t)__kernel_end);

    fmt_print("\n");
    uint16_t bus = 0;
    uint16_t device = 1;
    uint16_t function = 1;
    uint32_t header0 = pci_read_register(bus, device, function, 0);
    uint16_t vendor_id = (uint16_t)header0;
    uint16_t device_id = header0 >> 16;
    fmt_print("bus: %hu, device: %hhu\n", bus, device);
    fmt_print("vendor_id: %hx, device_id: %hx\n", vendor_id, device_id);
    uint32_t header2 = pci_read_register(bus, device, function, 8);
    uint8_t class_code = (header2 >> 24) & 0xFF;
    uint8_t subclass = (header2 >> 16) & 0xFF;
    uint8_t prog_if = (header2 >> 8) & 0xFF;
    uint8_t revision_id = header2 & 0xFF;
    fmt_print("class_code: %hhx, subclass: %hhx\n", class_code, subclass);
    fmt_print("prog_if: %hhx, revision_id: %hhx\n", prog_if, revision_id);
    uint32_t header3 = pci_read_register(bus, device, function, 12);
    uint8_t bist = (header3 >> 24) & 0xFF;
    uint8_t header_type = (header3 >> 16) & 0xFF;
    uint8_t latency_timer = (header3 >> 8) & 0xFF;
    uint8_t cache_line = header3 & 0xFF;
    fmt_print("bist: %hhx, header_type: %hhx\n", bist, header_type);
    fmt_print("latency_timer: %hhx, cache_line: %hhx\n", latency_timer, cache_line);
    uint32_t bar0 = pci_read_register(bus, device, function, 16);
    uint32_t bar1 = pci_read_register(bus, device, function, 20);
    uint32_t bar2 = pci_read_register(bus, device, function, 24);
    uint32_t bar3 = pci_read_register(bus, device, function, 28);
    uint32_t bar4 = pci_read_register(bus, device, function, 32);
    fmt_print("bar0: %x\n", bar0);
    fmt_print("bar1: %x\n", bar1);
    fmt_print("bar2: %x\n", bar2);
    fmt_print("bar3: %x\n", bar3);
    fmt_print("bar4: %x\n", bar4);

    fmt_print("\n");
    /*
    fmt_print("Before write:\n");
    ata_read_sector(IDE_BUS_PRIM, 0, 0, data);
    for (size_t i = 0; i < 512 / 32; i++) {
        for (size_t j = 0; j < 32; j++) {
            fmt_print("%hhx ", data[i * 32 + j]);
        }
        fmt_print("\n");
    }
    memory_fill(data, 512, 0xFF);
    ata_write_sector(IDE_BUS_PRIM, 0, 0, data);
    fmt_print("After write:\n");
    ata_read_sector(IDE_BUS_PRIM, 0, 0, data);
    for (size_t i = 0; i < 512 / 32; i++) {
        for (size_t j = 0; j < 32; j++) {
            fmt_print("%hhx ", data[i * 32 + j]);
        }
        fmt_print("\n");
    }
    */

    /*
    fmt_print("Attributes of hello.txt:\n");
    TagFS_File_Metadata file_meta;
    tfs_get_file_attributes(str_literal("hello.txt"), &file_meta);
    fmt_print("first_data_sector: %d\n", file_meta.first_data_sector);
    fmt_print("size: %d\n", file_meta.size);
    fmt_print("name: %s\n", file_meta.name);
    fmt_print("\n");

    fmt_print("Contents of hello.txt:\n");
    tfs_read(str_literal("hello.txt"), data, 512, 0);
    fmt_print("%s\n", data);

    fmt_print("Writing to hello.txt...\n");
    tfs_write(str_literal("hello.txt"), (uint8_t *)"hihihihihi\n", 11, 0);
    fmt_print("After write:\n");
    tfs_read(str_literal("hello.txt"), data, 512, 0);
    fmt_print("%s\n", data);

    fmt_print("\n");
    fmt_print("Attributes after writing:\n");
    tfs_get_file_attributes(str_literal("hello.txt"), &file_meta);
    fmt_print("first_data_sector: %d\n", file_meta.first_data_sector);
    fmt_print("size: %d\n", file_meta.size);
    fmt_print("name: %s\n", file_meta.name);
    fmt_print("\n");

    fmt_print("\n");
    fmt_print("Making a new file now...\n");
    tfs_make_new_file(str_literal("goodbye.txt"));
    tfs_get_file_attributes(str_literal("goodbye.txt"), &file_meta);
    fmt_print("first_data_sector: %d\n", file_meta.first_data_sector);
    fmt_print("size: %d\n", file_meta.size);
    fmt_print("name: %s\n", file_meta.name);
    fmt_print("\n");
    */

    TagFS_File_Metadata file_meta;
    tfs_get_file_attributes(str_literal("userspace"), &file_meta);
    uint8_t *data = memory_allocate(file_meta.size);
    tfs_read(str_literal("userspace"), data, file_meta.size, 0);

    ELF_File_Header *header = (ELF_File_Header *)data;
    fmt_print("Identifier:\n");
    fmt_print("  magic: %hhd %hhd %hhd %hhd\n",
        header->ident.magic[0], header->ident.magic[1], header->ident.magic[2], header->ident.magic[3]);
    fmt_print("  class: %hhd\n", header->ident.class);
    fmt_print("  data_encoding: %hhd\n", header->ident.data_encoding);
    fmt_print("  version: %hhd\n", header->ident.version);
    fmt_print("  os_abi: %hhd\n", header->ident.os_abi);
    fmt_print("  abi_version: %hhd\n", header->ident.abi_version);

    fmt_print("The rest:\n");
    fmt_print("  type: %hd\n", header->type);
    fmt_print("  machine: %hd\n", header->machine);
    fmt_print("  version: %d\n", header->version);
    fmt_print("  entry: %x\n", header->entry);
    fmt_print("  prog_header_offset: %d\n", header->prog_header_offset);
    fmt_print("  section_header_offset: %d\n", header->section_header_offset);
    fmt_print("  flags: %x\n", header->flags);
    fmt_print("  elf_header_size: %hd\n", header->elf_header_size);
    fmt_print("  prog_header_entry_size: %hd\n", header->prog_header_entry_size);
    fmt_print("  prog_header_count: %hd\n", header->prog_header_count);
    fmt_print("  section_header_entry_size: %hd\n", header->section_header_entry_size);
    fmt_print("  section_header_count: %hd\n", header->section_header_count);
    fmt_print("  section_header_names_index: %hd\n", header->section_header_names_index);
    fmt_print("\n");

    ELF32_Section_Header *section_header = (ELF32_Section_Header *)(data + header->section_header_offset);
    ELF32_Section_Header section_names_header = section_header[header->section_header_names_index];
    char *section_names = (char *)(data + section_names_header.offset);

    ELF32_Section_Header text_section_header = section_header[1];
    fmt_print("Text section header:\n");
    fmt_print("  name_index: %d\n", text_section_header.name);
    fmt_print("  name: %s\n", section_names + text_section_header.name);
    fmt_print("  type: %d\n", text_section_header.type);
    fmt_print("  flags: %x\n", text_section_header.flags);
    fmt_print("  address: %x\n", text_section_header.address);
    fmt_print("  offset: %d\n", text_section_header.offset);
    fmt_print("  size: %d\n", text_section_header.size);
    fmt_print("  link: %d\n", text_section_header.link);
    fmt_print("  info: %d\n", text_section_header.info);
    fmt_print("  address_align: %d\n", text_section_header.address_align);
    fmt_print("  entry_size: %d\n", text_section_header.entry_size);
    fmt_print("Flags:\n");
    fmt_print("  write: %hhd\n", text_section_header.flags.write);
    fmt_print("  alloc: %hhd\n", text_section_header.flags.alloc);
    fmt_print("  exec_instr: %hhd\n", text_section_header.flags.exec_instr);
    fmt_print("  merge: %hhd\n", text_section_header.flags.merge);
    fmt_print("  strings: %hhd\n", text_section_header.flags.strings);
    fmt_print("  info_link: %hhd\n", text_section_header.flags.info_link);
    fmt_print("  link_order: %hhd\n", text_section_header.flags.link_order);
    fmt_print("  os_nonconforming: %hhd\n", text_section_header.flags.os_nonconforming);
    fmt_print("  group: %hhd\n", text_section_header.flags.group);
    fmt_print("  thread_local_storage: %hhd\n", text_section_header.flags.thread_local_storage);
    fmt_print("  os: %hhd\n", text_section_header.flags.os);
    fmt_print("  proc: %hhd\n", text_section_header.flags.proc);
    fmt_print("\n");

    ELF32_Program_Header *program_header = (ELF32_Program_Header *)(data + header->prog_header_offset);
    fmt_print("Program header:\n");
    fmt_print("  offset: %d\n", program_header->offset);
    fmt_print("  virtual_address: %d\n", program_header->virtual_address);
    fmt_print("  physical_address: %d\n", program_header->physical_address);
    fmt_print("  file_size: %d\n", program_header->file_size);
    fmt_print("  memory_size: %d\n", program_header->memory_size);
    fmt_print("  flags: %d\n", program_header->flags);
    fmt_print("  align: %d\n", program_header->align);
    fmt_print("Flags:\n");
    fmt_print("  executable: %hhd\n", program_header->flags.executable);
    fmt_print("  writeable: %hhd\n", program_header->flags.writeable);
    fmt_print("  readable: %hhd\n", program_header->flags.readable);
    fmt_print("  proc: %hhd\n", program_header->flags.proc);


    ASSERT(header->ident.class == EC_32, "Unsupported ELF class.");
    ASSERT(header->ident.data_encoding == EDE_LSB, "Unsupported ELF data encoding.");
    memory_free(data);
    
    for(;;);
}
