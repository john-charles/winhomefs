#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <regex.h>
#include <fuse.h>
#include <fuse_opt.h>


#ifndef FS_H
#define FS_H

static int fs_getattr(const char *path, struct stat *stbuf);
static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi);


#endif