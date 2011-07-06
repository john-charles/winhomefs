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


#endif