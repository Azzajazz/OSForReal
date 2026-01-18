# TagFS

## Table of contents

1. [Overview](#overview)
2. [File system format](#format)
    1. [Boot sector](#boot-sector)
***TODO: Finish table of contents***
***TODO: Tips on writing drivers?***

## Overview<a name="overview"></a>

TagFS aims to be an on-disk tagged file system. In a tagged file system, there are no directories; all files exist in the root of the file system. Every file may have one or more tags associated with them and groups of files can be identified by querying tags. Both tags and files have metadata associated with them, defined in [Section 2](#format).
***TODO: give examples.***
***TODO: Expand on this after filling in the following sections***

## File system format<a name="format"></a>

***TODO: First pass at file system format***
- Associative array of `tag <-> file` (fixed size maybe)
- File metadata area (probably a FAT)
- File data area (probably a FAT)
***TODO: Merge the two above?***
- Tag metadata area (probably a FAT)
- Boot sector
- FS metadata block

|Boot sector|File metadata|Tag metadata|Tag-File map|FAT|Data|
|-----------|-------------|------------|------------|---|----|

### Boot sector

The boot sector is a single sector beginning with an FS metadata structure. The FS metadata has the following structure:

|Offset|Size|Name                    |Description                                        |
|:-----|:---|:-----------------------|:--------------------------------------------------|
|0     |4   |`jump`                  |Jump instruction.                                  |
|4     |4   |`version`               |Version number of the TagFS file system. Must be 1.|
|8     |4   |`sector_size`           |Size of a sector in bytes.                         |
|12    |4   |`sector_count`          |Number of sectors on the storage media.            |
|16    |4   |`file_meta_sector_count`|Size of the file metadata section in sectors.      |
|20    |4   |`tag_meta_sector_count` |Size of the tag metadata section in sectors.       |
|24    |4   |`tag_file_sector_count` |Size of the tag file map section in sectors.       |
|28    |4   |`fat_sector_count`      |Size of the FAT section in sectors.                |

**Size:** `sector size` bytes
**Byte Offset:** `0`

### File Metadata

The file metadata block contains file metadata entries of the following form:
|Offset|Size|Name               |Description                                                     |
|:-----|:---|:------------------|:---------------------------------------------------------------|
|2     |4   |`first_data_sector`|Offset of the first segment of data in the data section, plus 1.|
|6     |4   |`size`             |Size of the file in bytes.                                      |
|10    |22  |`name`             |The name of the file, including a trailing null byte.           |
***TODO: Probably need some other stuff here.***

The file id is defined to be the index of its file metadata entry in the file metadata block, plus 1. If `name` starts with a null byte, then the file metadata entry is empty. As a consequence, files cannot have an empty name.

**Size:** `32 * # of entries` bytes
**Byte Offset:** `sector size`

### Tag Metadata

The tag metadata block contains tag metadata entries of the following form:
|Offset|Size|Name               |Description                                                     |
|:-----|:---|:------------------|:---------------------------------------------------------------|
|0     |32  |`name`             |The name of the tag.                                            |

The tag id is defined to be the index of its tag metadata entry in the tag metadata block, plus 1. If `name` starts with a null byte, then the tag metadata entry is empty. As a consequence, tags cannot have an empty name.
***TODO: Probably need some other stuff here.***

**Size:** `32 * # of entries` bytes
**Byte Offset:** `(1 + file metadata sector count) * sector size`

### Tag File Map

The tag file map is an on-disk associative array between tag ids and file ids. Entries are stored contiguously from the start of the tag file map, with the first unoccupied entry marked with `tag_id == 0`. Each entry has the following structure:
|Offset|Size|Name     |Description |
|:-----|:---|:--------|:-----------|
|0     |2   |`tag_id` |The tag id. |
|2     |4   |`file_id`|The file id.|

**Size:** `# of sectors in tag file map * sector size` bytes
**Byte Offset:** `(1 + file metadata sector count + tag metadata sector count) * sector size`

### FAT + Data

The FAT and data sections are identical to FAT16. See [the FAT32 specification](https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf) for details.
