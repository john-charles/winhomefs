/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall `pkg-config fuse --cflags --libs` -lulockmgr fusexmp_fh.c -o fusexmp_fh
*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <fuse.h>
#include <fuse_opt.h>
#include <ulockmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "hidden.h"
#include "resolve.h"
#include "utilities.h"
#include "argsparse.h"
#include "initialization.h"

/** Global variables **/

char * root_path;
char * winversion;
char * winredirect;


  
list_t * hidden_list_home = 0;
list_t * hidden_list_user = 0;

redirect_t * my_documents;
redirect_t * my_music;
redirect_t * my_pictures;
redirect_t * my_videos;


static int xmp_getattr(const char *path, struct stat *stbuf){
  
  /* NOTE: use regex to update this to cope with magic files... */
  printf("...getattr: path is %s\n", path );
  int res;
  char * rp = resolve( path );
  res = lstat(rp, stbuf);
  free(rp);
  printf("...getattr: res = %i, errno=%i\n", res, errno );
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi){
	int res;

	(void) path;

	res = fstat(fi->fh, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask){
  int res;
  char * rp = resolve( path );
  
  res = access(rp, mask);
  
  free( rp );
  
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size){
  
  char * rp = resolve( path );
  int res;

  res = readlink(rp, buf, size - 1);
  
  free( rp );
  
  if (res == -1)return -errno;
  
  buf[res] = '\0';
  return 0;
  
}

static list_t * fs_readdir_loadroot( ){
  
  list_t * contents = list_t_new_listdir( root_path );
  list_t * dotfiles = list_t_new_listdir( winredirect );
  
  list_t_extend_unique( contents, dotfiles );
  list_t_free( dotfiles );
  
  return contents;
  
}

static list_t * fs_readdir_purge_hidden( const char * real_path, list_t * contents ){
  
  if( list_t_contains( contents, ".show-hidden" ) ||
      list_t_contains( contents, ".show_hidden" ) ){
    
    return contents;
    
  }
  
  list_t * hidden = get_hidden_list( real_path );
  list_t * result = list_t_new();

  int i, hidden_found = 0;
  
  for( i = 0; i < contents->length; i++ ){
    
    printf("...purge_hidden: is file %s hidden?\n", contents->data[i] );
    
    if( !list_t_contains( hidden, contents->data[i] ) ){
      printf("...purge_hidden: file %s is not hidden!\n", contents->data[i] );
      list_t_append_unique( result, contents->data[i] );
      
    } else {
      printf("...purge_hidden: file %s is hidden!\n", contents->data[i] );
      hidden_found = hidden_found + 1;
      
    }
    
    if( list_t_contains( result, contents->data[i] ) ){
      printf("...purge_hidden: file %s is in result!\n", contents->data[i] );
    } else {
      printf("...purge_hidden: file %s is NOT in result!\n", contents->data[i] );
    }
      
    
  }
  
  if( hidden_found > 0 ){
    
    char * count_name = (char*)malloc( 50 );
    sprintf( count_name, ". %i hidden files found", hidden_found );
    list_t_append( result, count_name );
    free( count_name );
    
  }
  
  list_t_free( hidden );
  list_t_free( contents );
  
  return result;
  
}
  
typedef struct {
  
  list_t * contents;
  off_t    offset;
  
} directory_t;

static int fs_opendir(const char * path, struct fuse_file_info * info ){
  
  /* TODO Make sure that this all compiles and works.... */
  char        * real_path = resolve( path );
  directory_t * directory = malloc( sizeof( directory_t ) );
  
  if( strcmp( path, "/" ) == 0 ){
    
    directory->contents = fs_readdir_loadroot();
    
  } else {
    
    directory->contents = list_t_new_listdir( real_path );
    
  }
  
  directory->contents = fs_readdir_purge_hidden( real_path, directory->contents );  
  directory->offset = 0;
  
  info->fh = (unsigned long) directory;
  
  return 0;
  
}
    
    
    
    
  
  

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
  
  //NOTE: fix this so that any return to offset 0 causes a re-read of the dir contents...
  
  directory_t * directory = (directory_t *) (uintptr_t) fi->fh;
  
//   list_t_print( entries );
  
  int i;
  
  for( i = offset; i < directory->contents->length; i++ ){
    printf("...readdir: filling in file %s\n", directory->contents->data[i] );
    filler( buf, directory->contents->data[i], 0, i+1 );
    
  }
  
  directory->offset = i;
  
  return 0;
  
}

static int fs_releasedir(const char * path, struct fuse_file_info * fi){
  
  directory_t * directory = (directory_t *) (uintptr_t) fi->fh;
  
  list_t_free( directory->contents );
  
  free( directory );
  
  return 0;
  
}
  
  


static int xmp_mknod(const char * ipath, mode_t mode, dev_t rdev)
{
    char * path = resolve( ipath );
	int res;

	if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1){
      free( path );
      return -errno;
    }
  free( path );
	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode){
  
  int res;
  char * real_path = resolve( path );
  
  res = mkdir( real_path, mode );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int xmp_unlink(const char *path){
  
  if( strcmp( path, "/AppData" ) == 0 ) return -EPERM;
  if( strcmp( path, "/Application Data") == 0 ) return -EPERM;
  if( strcmp( path, "/Desktop" ) == 0 ) return -EPERM;
  if( strcmp( path, "/Downloads" ) == 0 ) return -EPERM;
  if( strcmp( path, "/Documents" ) == 0 ) return -EPERM;
  if( strcmp( path, "/Music" ) == 0 ) return -EPERM;
  if( strcmp( path, "/Pictures" ) == 0 ) return -EPERM;
  if( strcmp( path, "/Projects" ) == 0 ) return -EPERM;
  if( strcmp( path, "/Videos" ) == 0 ) return -EPERM;
  
  int res;
  char * real_path = resolve( path );
  
  res = unlink(real_path);
  
  free( real_path );
  
  if (res == -1) return -errno;
  return 0;
  
}

static int xmp_rmdir(const char *path){
  
  if( strcmp( path, "/AppData" ) == 0 )         return -EPERM;
  if( strcmp( path, "/Application Data") == 0 ) return -EPERM;
  if( strcmp( path, "/Contacts" ) == 0 )        return -EPERM;
  if( strcmp( path, "/Desktop" ) == 0 )         return -EPERM;
  if( strcmp( path, "/Downloads" ) == 0 )       return -EPERM;
  if( strcmp( path, "/Documents" ) == 0 )       return -EPERM;
  if( strcmp( path, "/Favorites" ) == 0 )       return -EPERM;
  if( strcmp( path, "/Links" ) == 0 )           return -EPERM;
  if( strcmp( path, "/Music" ) == 0 )           return -EPERM;
  if( strcmp( path, "/Pictures" ) == 0 )        return -EPERM;
  if( strcmp( path, "/Projects" ) == 0 )        return -EPERM;
  if( strcmp( path, "/Saved Games" ) == 0 )     return -EPERM;
  if( strcmp( path, "/Videos" ) == 0 )          return -EPERM;
  
  int res;
  char * real_path = resolve( path );
  
  res = rmdir( real_path );
  
  free( real_path );
  
  if (res == -1) return -errno;
  return 0;
  
}

static int xmp_symlink(const char *from, const char *to){
  
  char * real_to = resolve( to );
  int res;
  printf("winhomefs.c: xmp_symlink: from = %s to = %s\n", from, to );
  printf("winhomefs.c: xmp_symlink: real_to = %s\n", real_to );
  /* NOTE: I have not yet decided how I want to my symlinks work... */
  res = symlink( from, real_to );
  
  free( real_to );
  
  if (res == -1) return -errno;

  return 0;
}

static int xmp_rename(const char *from, const char *to){
  
  char * real_from = resolve( from );
  char * real_to   = resolve( to );
  int res;
  
  res = rename( real_from, real_to );
  
  free( real_from );
  free( real_to );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int xmp_link(const char *from, const char *to){
  
  char * real_from = resolve( from );
  char * real_to   = resolve( to );
  int res;
  
  res = link( real_from, real_to );
  
  free( real_from );
  free( real_to );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int xmp_chmod(const char *path, mode_t mode){
  
  char * real_path = resolve( path );
  int res;
  
  res = chmod( real_path, mode );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid){
  
  char * real_path = resolve( path );
  int res;
  
  res = lchown(path, uid, gid);
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int xmp_truncate(const char *path, off_t size){
  
  char * real_path = resolve( path );
  int res;
  
  res = truncate( real_path, size );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
  
}

static int xmp_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	int res;

	(void) path;

	res = ftruncate(fi->fh, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;
    
    char * real_path = resolve( path );

	res = utimes( real_path, tv );
    
    free( real_path );
    
	if (res == -1)return -errno;
    return 0;
    
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	int fd;
  char * real_path = resolve( path );
	fd = open(real_path, fi->flags, mode);
  free( real_path );
	if (fd == -1)
		return -errno;

	fi->fh = fd;
	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi){
  
  int fd;
  char * real_path = resolve( path );
  
  fd = open( real_path, fi->flags );
  
  free( real_path );
  
  if (fd == -1)return -errno;
  
  fi->fh = fd;
  
  return 0;
  
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int res;

	(void) path;
	res = pread(fi->fh, buf, size, offset);
	if (res == -1)
		res = -errno;

	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int res;

	(void) path;
	res = pwrite(fi->fh, buf, size, offset);
	if (res == -1)
		res = -errno;

	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf){
  
  int res;
  char * real_path = resolve( path );
  
  res = statvfs( real_path, stbuf );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int xmp_flush(const char *path, struct fuse_file_info *fi)
{
	int res;

	(void) path;
	/* This is called from every close on an open file, so call the
	   close on the underlying filesystem.	But since flush may be
	   called multiple times for an open file, this must not really
	   close the file.  This is important if used on a network
	   filesystem like NFS which flush the data/metadata on close() */
	res = close(dup(fi->fh));
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	close(fi->fh);

	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	int res;
	(void) path;

#ifndef HAVE_FDATASYNC
	(void) isdatasync;
#else
	if (isdatasync)
		res = fdatasync(fi->fh);
	else
#endif
		res = fsync(fi->fh);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, const char *value, size_t size, int flags){
  
  char * real_path = resolve( path );
  int res = lsetxattr( real_path, name, value, size, flags );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int xmp_getxattr(const char *path, const char *name, char *value, size_t size){
  
  char * real_path = resolve( path );
  int res = lgetxattr( real_path, name, value, size );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return res;
  
}

static int xmp_listxattr(const char *path, char *list, size_t size){
  
  char * real_path = resolve( path );
  int res = llistxattr( real_path, list, size );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return res;
  
}

static int xmp_removexattr(const char *path, const char *name){
  
  char * real_path = resolve( path );
  int res = lremovexattr( real_path, name );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}
#endif /* HAVE_SETXATTR */

static int xmp_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock){
  
  (void) path;
  
  return ulockmgr_op(fi->fh, cmd, lock, &fi->lock_owner, sizeof(fi->lock_owner));
  
}

static struct fuse_operations xmp_oper = {
  
  .getattr  = xmp_getattr,
  .fgetattr = xmp_fgetattr,
  .access   = xmp_access,
  .readlink = xmp_readlink,
  .opendir  = fs_opendir,
  .readdir  = xmp_readdir,
  .releasedir = fs_releasedir,
  
  .mknod		= xmp_mknod,
  .mkdir		= xmp_mkdir,
  .symlink	= xmp_symlink,
  .unlink		= xmp_unlink,
  .rmdir		= xmp_rmdir,
  .rename		= xmp_rename,
  .link		= xmp_link,
  .chmod		= xmp_chmod,
  .chown		= xmp_chown,
  .truncate	= xmp_truncate,
  .ftruncate	= xmp_ftruncate,
  .utimens	= xmp_utimens,
  .create		= xmp_create,
  .open		= xmp_open,
  .read		= xmp_read,
  .write		= xmp_write,
  .statfs		= xmp_statfs,
  .flush		= xmp_flush,
  .release	= xmp_release,
  .fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
  .setxattr	= xmp_setxattr,
  .getxattr	= xmp_getxattr,
  .listxattr	= xmp_listxattr,
  .removexattr	= xmp_removexattr,
#endif
  .lock		= xmp_lock,

  .flag_nullpath_ok = 1,
};

int main(int argc, char *argv[])
{
  
  int i;
  
  for( i = 0; i < argc; i++ ){
    printf("%i - \"%s\"\n", i, argv[i] );
  }
  
//   return 0;

//   list_t * d = list_t_new_listdir("/mnt/Personal/Users/john-charles/Music/Collection/Compilations/");
//   
//   for( i = 0; i < d->length; i++ ){
//     
//     puts( d->data[i] );
//     
//   }
  
  int success = 1;  
  list_t * fuse_args = list_t_new();
  char * prospective_root = preparse_opts( &argc, argv, fuse_args );
   
  success = success && initialize_environment( prospective_root );
  success = success && initialize_redirect( );
  success = success && initialize_regex( );
  success = success && initialize_default_hidden_lists();
  
//   puts( resolve( "/Music/Northern Exposure: III/Track 1.mp3" ) );
//   return 0;
  
  if( success ){
  
    
    success = fuse_main( fuse_args->length, fuse_args->data, &xmp_oper, NULL);
    
    
    
  }

  return success;
}
