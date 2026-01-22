#define FUSE_USE_VERSION 31
#include <fuse.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>

#include "utils.c"

uint8_t *mapped_img = NULL;

int tagfs_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    int file_meta_count = fs_meta->file_meta_sector_count * fs_meta->sector_size / sizeof(File_Metadata);

    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
        st->st_nlink = 0;
        for (int i = 0; i < file_meta_count; ++i) {
            if (file_meta[i].name[0] != '\0') {
                st->st_size += file_meta[i].size;
            }
        }

        return 0;
    }
    else {
        st->st_mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
        st->st_nlink = 0;
        for (int i = 0; i < file_meta_count; ++i) {
            if (strcmp(file_meta[i].name, path + 1) == 0) {
                st->st_size += file_meta[i].size;
                return 0;
            }
        }

        return -ENOENT;
    }
}

int tagfs_readdir(
    const char *path,
    void *buf,
    fuse_fill_dir_t filler,
	off_t offset,
    struct fuse_file_info *fi,
    enum fuse_readdir_flags flags
) {
    (void)offset;
    (void)fi;
    (void)flags;

    if (strcmp(path, "/") == 0) {
        FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
        File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
        int file_meta_count = fs_meta->file_meta_sector_count * fs_meta->sector_size / sizeof(File_Metadata);
        for (int i = 0; i < file_meta_count; ++i) {
            if (file_meta[i].name[0] != '\0') {
                struct stat file_stat = {
                    .st_size = file_meta[i].size,
                    .st_mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO,
                };
                filler(buf, file_meta[i].name, &file_stat, 0, 0);
            }
        }
    }
    return 0;
}

static int tagfs_read(
    const char *path,
    char *buf,
    size_t size,
    off_t offset,
    struct fuse_file_info *fi
) {
    path = path + 1; // Skip the '/' at the start.

    FS_Metadata *fs_meta = (FS_Metadata*)mapped_img;
    File_Metadata *file_meta = get_file_metadata(mapped_img, fs_meta);
    int file_meta_count = fs_meta->file_meta_sector_count * fs_meta->sector_size / sizeof(File_Metadata);

    // Find the file id.
    uint16_t file_id = 0;
    File_Metadata *this_file_meta = NULL;
    for (; file_id < file_meta_count; file_id++) {
        if (strcmp(path, file_meta[file_id].name) == 0) {
            this_file_meta = &file_meta[file_id];
            file_id++;
            break;
        }
    }

    if (this_file_meta == NULL) {
        return -ENOENT;
    }

    if (offset >= this_file_meta->size) {
        return 0;
    }

    int sector_skip_count = offset / fs_meta->sector_size;
    int byte_skip_count = offset % fs_meta->sector_size;

    // Skip the number of sectors specified by the offset.
    uint16_t *fat = get_fat(mapped_img, fs_meta);
    int fat_index = this_file_meta->first_data_sector;
    for (; sector_skip_count > 0; sector_skip_count--) {
        if (fat[fat_index] == 0xffff) {
            return -EBADFD;
        }
        fat_index = fat[fat_index];
    }

    uint8_t *data = get_data(mapped_img, fs_meta);
    if (size <= fs_meta->sector_size - byte_skip_count) {
        // We're just reading from one sector.
        memcpy(buf, data + fs_meta->sector_size * fat_index + byte_skip_count, size);
        return size;
    }
    else {
        size_t buf_index = 0;

        // We're reading multiple sectors. Read as much of the current sector as we can.
        size_t first_sector_bytes = fs_meta->sector_size - byte_skip_count;
        memcpy(buf, data + fs_meta->sector_size * fat_index + byte_skip_count, first_sector_bytes);
        buf_index += first_sector_bytes;

        // Now read the rest.
        while (buf_index < size) {
            if (buf_index >= this_file_meta->size) {
                return buf_index;
            }
            fat_index = fat[fat_index];

            size_t bytes_to_read = fs_meta->sector_size;
            if (size <= buf_index + fs_meta->sector_size) {
                bytes_to_read = size - buf_index;
            }

            memcpy(buf + buf_index, data + fs_meta->sector_size * fat_index, bytes_to_read);
            buf_index += bytes_to_read;
        }
        return buf_index;
    }
}

static struct fuse_operations myfs_ops = {
    .getattr = tagfs_getattr,
    .readdir = tagfs_readdir,
    .read = tagfs_read,
};

int main(int argc, char **argv)
{
    char *image_name = "driver.img";
    int fd = open(image_name, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: Could not open %s: %s\n", image_name, strerror(errno));
        return 1;
    }

    FS_Metadata fs_meta;
    read_fs_metadata(&fs_meta, fd);
    mapped_img = mmap(NULL, fs_meta.sector_count * fs_meta.sector_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    return fuse_main(argc, argv, &myfs_ops, NULL);
}
