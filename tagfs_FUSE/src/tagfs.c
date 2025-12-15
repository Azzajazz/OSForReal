#define FUSE_USE_VERSION 31
#include <fuse.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int tagfs_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
    if (!strcmp(path, "/")) {
        // it's the root directory (just an example, you probably have more directories)
        st->st_mode = S_IFDIR | 0755; // access rights and directory type
        st->st_nlink = 2;             // number of hard links, for directories this is at least 2
        st->st_size = 4096;           // file size
    } else {
        st->st_mode = S_IFREG | 0644; // access rights and regular file type
        st->st_nlink = 2;             // number of hard links
        st->st_size = 4096;           // file size
    }
    // user and group. we use the user's id who is executing the FUSE driver
    st->st_uid = getuid();
    st->st_gid = getgid();
    return 0;
}

static struct fuse_operations myfs_ops = {
    .getattr=tagfs_getattr,
};

char *devfile = NULL;

int main(int argc, char **argv)
{
    int i;

    // get the device or image filename from arguments
    for (i = 1; i < argc && argv[i][0] == '-'; i++);
    if (i < argc) {
        devfile = realpath(argv[i], NULL);
        memcpy(&argv[i], &argv[i+1], (argc-i) * sizeof(argv[0]));
        argc--;
    }

    // leave the rest to FUSE
    return fuse_main(argc, argv, &myfs_ops, NULL);
}

