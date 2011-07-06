#ifndef FS_READDIR_H
#define FS_READDIR_H

#include <fuse.h>

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi);

#endif