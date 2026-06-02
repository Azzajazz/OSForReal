typedef enum : uint16_t {
    ET_NONE,
    ET_RELOCATABLE,
    ET_EXECUTABLE,
    ET_DYNAMIC,
    ET_CORE,
    ET_LOWPROC = 0xff00,
    ET_HIGHPROC = 0xffff,
} ELF_Type;

typedef enum : uint16_t {
    EM_NONE = 0,
    EM_SPARC = 2,
    EM_386 = 3,
    EM_SPARC32PLUS = 18,
    EM_SPARCV9 = 43,
    EM_AMD64 = 62,
} ELF_Machine;

typedef enum : uint8_t {
    EC_NONE,
    EC_32,
    EC_64,
} ELF_Class;

typedef enum : uint8_t {
    EDE_NONE,
    EDE_LSB,
    EDE_MSB,
} ELF_Data_Encoding;

typedef struct {
    uint8_t magic[4];
    ELF_Class class;
    ELF_Data_Encoding data_encoding;
    uint8_t version;
    uint8_t os_abi;
    uint8_t abi_version;
    uint8_t __padding[7];
} ELF_Identifier;

typedef struct {
    ELF_Identifier ident;
    ELF_Type type;
    ELF_Machine machine;
    uint32_t version;
    uint32_t entry;
    uint32_t prog_header_offset;
    uint32_t section_header_offset;
    uint32_t flags;
    uint16_t elf_header_size;
    uint16_t prog_header_entry_size;
    uint16_t prog_header_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_count;
    uint16_t section_header_names_index;
} ELF_File_Header;

typedef enum : uint32_t {
    ESHT_NULL,
    ESHT_PROGBITS,
    ESHT_SYMTAB,
    ESHT_STRTAB,
    ESHT_RELA,
    ESHT_HASH,
    ESHT_DYNAMIC,
    ESHT_NOTE,
    ESHT_NOBITS,
    ESHT_REL,
    ESHT_SHLIB,
    ESHT_DYNSYM,
    ESHT_INIT_ARRAY,
    ESHT_FINI_ARRAY,
    ESHT_PREINIT_ARRAY,
    ESHT_GROUP,
    ESHT_SYMTAB_SHNDX,

    ESHT_LOWOS = 0x60000000,
    ESHT_HIGHOS = 0x6fffffff,

    ESHT_LOWPROC = 0x70000000,
    ESHT_HIGHPROC = 0x7fffffff,

    ESHT_LOWUSER = 0x80000000,
    ESHT_HIGHUSER = 0xffffffff,
} ELF_Section_Header_Type;

typedef struct {
    uint32_t write                : 1;
    uint32_t alloc                : 1;
    uint32_t exec_instr           : 1;
    uint32_t __unused1            : 1;
    uint32_t merge                : 1;
    uint32_t strings              : 1;
    uint32_t info_link            : 1;
    uint32_t link_order           : 1;
    uint32_t os_nonconforming     : 1;
    uint32_t group                : 1;
    uint32_t thread_local_storage : 1;
    uint32_t __unused2            : 9;
    uint32_t os                   : 8;
    uint32_t proc                 : 4;
} ELF_Section_Flags;

typedef struct {
    uint32_t name;
    ELF_Section_Header_Type type;
    ELF_Section_Flags flags;
    uint32_t address;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t address_align;
    uint32_t entry_size;
} ELF32_Section_Header;

typedef enum : uint32_t {
    EPHT_NULL,
    EPHT_LOAD,
    EPHT_DYNAMIC,
    EPHT_INTERP,
    EPHT_NOTE,
    EPHT_SHLIB,
    EPHT_PHDR,
    EPHT_TLS,

    EPHT_LOWOS = 0x60000000,
    EPHT_HIGHOS = 0x6fffffff,

    EPHT_LOWPROC = 0x70000000,
    EPHT_HIGHPROC = 0x7fffffff,
} ELF_Program_Header_Type;

typedef struct {
    uint32_t executable : 1;
    uint32_t writeable  : 1;
    uint32_t readable   : 1;
    uint32_t __unused1  : 25;
    uint32_t proc       : 4;
} ELF_Program_Flags;

typedef struct {
    ELF_Program_Header_Type type;
    uint32_t offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t file_size;
    uint32_t memory_size;
    ELF_Program_Flags flags;
    uint32_t align;
} ELF32_Program_Header;
