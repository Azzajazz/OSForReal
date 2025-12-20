# TagFS

## Table of contents

1. [Overview](#overview)
2. [File system format](#format)
3. [Behaviour](#behaviour) ***TODO: Rename this section***

## Overview<a name="overview"></a>

TagFS aims to be an on-disk tagged file system. In a tagged file system, there are no directories; all files exist in the root of the file system. Every file may have one or more tags associated with them and groups of files can be identified by querying tags. Both tags and files have metadata associated with them such as ...
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

-----------------------------------------------------------------------------------------------
|Boot sector|FS metadata|File metadata FAT?|File data FAT?|Tag metadata FAT?|Tag-File map|Data|
-----------------------------------------------------------------------------------------------

## Behaviour

***TODO: Fill this in***
- What happens on duplicate file names? Do files even have names?
