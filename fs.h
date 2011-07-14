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

static int fs_access(const char *path, int mask);
static int fs_chmod(const char *path, mode_t mode);
static int fs_chown(const char *path, uid_t uid, gid_t gid);
static int fs_fsync(const char *path, int isdatasync,  struct fuse_file_info *fi);
static int fs_getattr(const char *path, struct stat *stbuf);
static int fs_link(const char *from, const char *to);
static int fs_mkdir(const char *path, mode_t mode);
static int fs_mknod(const char *path, mode_t mode, dev_t rdev);
static int fs_open(const char *path, struct fuse_file_info *fi);
static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int fs_readlink(const char *path, char *buf, size_t size);
static int fs_release(const char *path, struct fuse_file_info *fi);
static int fs_rename(const char *from, const char *to);
static int fs_rmdir(const char *path);

static int fs_unlink(const char *path);

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi);



#endif