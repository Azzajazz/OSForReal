# TagFS

## Table of contents

1. [Overview](#overview)
2. [File system format](#format)
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

|Boot sector|FS metadata|File metadata|Tag metadata|Tag-File map|FAT|Data|
|-----------|-----------|-------------|------------|------------|---|----|

### Boot sector

The boot sector contains only the bootloader. TagFS does not store any file system data in this block.

**Size:** `512` bytes
**Byte Offset:** `0`

### FS Metadata

The FS metadata block takes up the next sector and has the following structure:

|Offset|Size|Name                    |Description                                        |
|:-----|:---|:-----------------------|:--------------------------------------------------|
|0     |4   |`version`               |Version number of the TagFS file system. Must be 1.|
|4     |4   |`sector_size`           |Size of a sector in bytes.                         |
|8     |4   |`file_meta_sector_count`|Size of the file metadata section in sectors.      |
|12    |4   |`tag_meta_sector_count` |Size of the tag metadata section in sectors.       |
|16    |4   |`tag_file_sector_count` |Size of the tag file map section in sectors.       |
|20    |4   |`fat_size_sector_count` |Size of the FAT section in sectors.                |

**Size:** `24` bytes
**Byte Offset:** `512`

### File Metadata

The file metadata block contains file metadata entries of the following form:
|Offset|Size|Name         |Description                                            |
|:-----|:---|:---         |:----------                                            |
|0     |2   |`id`         |The unique identifier of the file.                     |
|2     |4   |`data_offset`|Offset of the first segment of data in the data section|
|6     |26  |`name`       |The name of the file.                                  |
***TODO: Probably need some other stuff here.***

The `id` field must be unique over all file metadata entries. Duplicate file names are supported by TagFS, but the file system driver may enforce unique file names if desired.

**Size:** `32 * # of entries` bytes
**Byte Offset:** `512 + FS metadata sector count * sector size`

### Tag Metadata

The tag metadata block contains tag metadata entries of the following form:
|Offset|Size|Name         |Description                                            |
|:-----|:---|:---         |:----------                                            |
|0     |2   |`id`         |The unique identifier of the tag.                      |
|2     |4   |`data_offset`|Offset of the first segment of data in the data section|
|6     |26  |`name`       |The name of the tag.                                   |
***TODO: Probably need some other stuff here.***

**Size:** `32 * # of entries` bytes
**Byte Offset:** `512 + (FS metadata sector count + file metadata sector count) * sector size`

### Tag File Map

The tag file map is an on-disk associative array between tag ids and file ids. Each entry has the following structure:
|Offset|Size|Name     |Description |
|:-----|:---|:--------|:-----------|
|0     |2   |`tag_id` |The tag id. |
|2     |4   |`file_id`|The file id.|

**Size:** `# of sectors in tag file map * sector size` bytes
**Byte Offset:** `512 + (FS metadata sector count + file metadata sector count + tag metadata sector count) * sector size`
