/*
* WINHOMEFS This is a filesystem use for mouting
* and sharing a windows home directory with a linux
* instance without creating the big mess.
* 
* This file system is written by John-Charles D. Sokolow
* Copyright (C) 2011-1012 John-Charles D. Sokolow <john.charles.sokolow@gmail.com>
* 
* Based heavily on the example fuse file system "fusefs_fh.c"
* Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
* 
*
* License:  This  program  is  free  software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published by
* the  Free Software Foundation; Version 2 This program is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
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
#include <libgen.h>
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


static int fs_getattr(const char *path, struct stat *stbuf){
  
    int res;
    
    char * rp = resolve( path );
    char * rd = dirname( cat( rp, "", 0));
    
    memset( stbuf, 0, sizeof(*stbuf));
    
    if ( endswith( rp, ".fs-info")){
        
        lstat( rd, stbuf );
        
        stbuf->st_mode = 0;
        stbuf->st_mode = stbuf->st_mode | S_IFREG;
        stbuf->st_mode = stbuf->st_mode | S_IRUSR;        
        
        res = 0;
        
    } else if( endswith( rp, "/..." )){
        
        lstat( rd, stbuf );
        
        stbuf->st_mode = S_IFLNK;
        stbuf->st_mode = stbuf->st_mode | S_IRWXU | S_IRWXG | S_IRWXO;
        
        res = 0;
        
    } else {
        
        res = lstat(rp, stbuf);
        
    }
    
    free(rp);
    free(rd);
    
    if (res == -1) return -errno;
    return 0;
}

static int fs_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi){
    
    int res;

    (void) path;

    res = fstat(fi->fh, stbuf);
    
    if (res == -1) return -errno;
    return 0;
}

static int fs_access(const char *path, int mask){
  
    int res;
    char * rp = resolve( path );
    
    res = access(rp, mask);
    
    free( rp );
    
    if (res == -1) return -errno;
    return 0;
    
}


static int fs_readlink(const char *path, char *buf, size_t size){
    
    int res = 0;
    
    char * rp = resolve( path );
    
    if( endswith( path, "/..." ) ){
        
        rp = dirname( rp );
        
        // Note this should be safe to use here....
        strcpy( buf, rp );
        size = strlen( rp );
        
    } else {
        
        res = readlink(rp, buf, size - 1);
        if( res != -1 ){
            buf[res] = '\0';
        }
        
    }
    
    free( rp );
    
    if (res == -1)return -errno;
    return 0;
  
}

static list_t * get_root_contents( list_t * contents ){
  
  list_t * new_contents = list_t_new();
  
  char * elem = 0;
  int i = 0;
  for( i = 0; i < contents->length; i++ ){
    
    elem = contents->data[i];
    if ( elem[0] != '.' ){
      list_t_append( new_contents, elem );
    }
  }
  
  list_t_free( contents );
  
  return new_contents;
  
}
      
    
    

static list_t * fs_readdir_loadroot( ){
  
  list_t * contents = list_t_new_listdir( root_path );
  list_t * dotfiles = list_t_new_listdir( winredirect );
  
  contents = get_root_contents( contents );
  
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

  int i;
  
  for( i = 0; i < contents->length; i++ ){
    
    if( !list_t_contains( hidden, contents->data[i] ) ){
      
      list_t_append_unique( result, contents->data[i] );
      
    }   
    
  }
  
  list_t_append_unique( result, ".fs-info" );
  
  list_t_free( hidden );
  list_t_free( contents );
  
  return result;
  
}
  
typedef struct {
  
  char   * path;
  list_t * contents;
  off_t    offset;
  
} directory_t;

static int real_readdir(directory_t * directory){
    
    char * real_path = resolve(directory->path);
    
    if(directory->contents){
        /** if there is pre-existing contents, we need to free them first! **/
        list_t_free(directory->contents);
    }
    
    if( strcmp(directory->path, "/" ) == 0 ){
    
        directory->contents = fs_readdir_loadroot();
        
    } else {
        
        directory->contents = list_t_new_listdir( real_path );
        
    }
    
    directory->contents = fs_readdir_purge_hidden( real_path, directory->contents );
    list_t_append( directory->contents, "..." );
    directory->offset = 0;
    
    free(real_path);
    
    return 0;
    
}    
    

static int fs_opendir(const char * path, struct fuse_file_info * info ){
  
  directory_t * directory = malloc(sizeof(directory_t));
  memset(directory, 0, sizeof(directory));
  directory->path = (char*)malloc(strlen(path) + 1);
  strcpy(directory->path, path);
  
  real_readdir(directory); 
  
  info->fh = (unsigned long) directory;
  
  return 0;
  
}
    
    
    
    
  
  

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
  
  directory_t * directory = (directory_t *) (uintptr_t) fi->fh;
  int i;
  
  if(offset == 0 && directory->offset > 0){      
      real_readdir(directory);      
  }
    
  for( i = offset; i < directory->contents->length; i++ ){
    filler( buf, directory->contents->data[i], 0, i+1 );    
  }
  
  directory->offset = i;  
  return 0;
  
}

static int fs_releasedir(const char * path, struct fuse_file_info * fi){
  
  directory_t * directory = (directory_t *) (uintptr_t) fi->fh;
  
  list_t_free(directory->contents);
  free(directory->path);
  free(directory);
  
  return 0;
  
}
  
  


static int fs_mknod(const char * ipath, mode_t mode, dev_t rdev)
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

static int fs_mkdir(const char *path, mode_t mode){
  
  int res;
  char * real_path = resolve( path );
  
  res = mkdir( real_path, mode );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int fs_unlink(const char *path){
  
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

static int fs_rmdir(const char *path){
  
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

static int fs_symlink(const char *from, const char *to){
  
  char * real_to = resolve( to );
  int res;
  printf("winhomefs.c: fs_symlink: from = %s to = %s\n", from, to );
  printf("winhomefs.c: fs_symlink: real_to = %s\n", real_to );
  /* NOTE: I have not yet decided how I want to my symlinks work... */
  res = symlink( from, real_to );
  
  free( real_to );
  
  if (res == -1) return -errno;

  return 0;
}

static int fs_rename(const char *from, const char *to){
  
  char * real_from = resolve( from );
  char * real_to   = resolve( to );
  int res;
  
  res = rename( real_from, real_to );
  
  free( real_from );
  free( real_to );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int fs_link(const char *from, const char *to){
  
  char * real_from = resolve( from );
  char * real_to   = resolve( to );
  int res;
  
  res = link( real_from, real_to );
  
  free( real_from );
  free( real_to );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int fs_chmod(const char *path, mode_t mode){
  
  char * real_path = resolve( path );
  int res;
  
  res = chmod( real_path, mode );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int fs_chown(const char *path, uid_t uid, gid_t gid){
  
  char * real_path = resolve( path );
  int res;
  
  res = lchown(path, uid, gid);
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int fs_truncate(const char *path, off_t size){
  
  char * real_path = resolve( path );
  int res;
  
  res = truncate( real_path, size );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
  
}

static int fs_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	int res;

	(void) path;

	res = ftruncate(fi->fh, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int fs_utimens(const char *path, const struct timespec ts[2])
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

static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
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

static int fs_open(const char *path, struct fuse_file_info *fi){
  
  int fd;
  char * real_path = resolve( path );
  
  fd = open( real_path, fi->flags );
  
  free( real_path );
  
  if (fd == -1)return -errno;
  
  fi->fh = fd;
  
  return 0;
  
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int res;

	(void) path;
	res = pread(fi->fh, buf, size, offset);
	if (res == -1)
		res = -errno;

	return res;
}

static int fs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int res;

	(void) path;
	res = pwrite(fi->fh, buf, size, offset);
	if (res == -1)
		res = -errno;

	return res;
}

static int fs_statfs(const char *path, struct statvfs *stbuf){
  
  int res;
  char * real_path = resolve( path );
  
  res = statvfs( real_path, stbuf );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int fs_flush(const char *path, struct fuse_file_info *fi)
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

static int fs_release(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	close(fi->fh);

	return 0;
}

static int fs_fsync(const char *path, int isdatasync,
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
static int fs_setxattr(const char *path, const char *name, const char *value, size_t size, int flags){
  
  char * real_path = resolve( path );
  int res = lsetxattr( real_path, name, value, size, flags );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}

static int fs_getxattr(const char *path, const char *name, char *value, size_t size){
  
  char * real_path = resolve( path );
  int res = lgetxattr( real_path, name, value, size );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return res;
  
}

static int fs_listxattr(const char *path, char *list, size_t size){
  
  char * real_path = resolve( path );
  int res = llistxattr( real_path, list, size );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return res;
  
}

static int fs_removexattr(const char *path, const char *name){
  
  char * real_path = resolve( path );
  int res = lremovexattr( real_path, name );
  
  free( real_path );
  
  if (res == -1)return -errno;
  return 0;
  
}
#endif /* HAVE_SETXATTR */

static int fs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock){
  
  (void) path;
  //return -ENOSYS; 
  return ulockmgr_op(fi->fh, cmd, lock, &fi->lock_owner, sizeof(fi->lock_owner));
  
}

static struct fuse_operations fs_oper = {
  
  .getattr  = fs_getattr,
  .fgetattr = fs_fgetattr,
  .access   = fs_access,
  .readlink = fs_readlink,
  .opendir  = fs_opendir,
  .readdir  = fs_readdir,
  .releasedir = fs_releasedir,
  
  .mknod		= fs_mknod,
  .mkdir		= fs_mkdir,
  .symlink	= fs_symlink,
  .unlink		= fs_unlink,
  .rmdir		= fs_rmdir,
  .rename		= fs_rename,
  .link		= fs_link,
  .chmod		= fs_chmod,
  .chown		= fs_chown,
  .truncate	= fs_truncate,
  .ftruncate	= fs_ftruncate,
  .utimens	= fs_utimens,
  .create		= fs_create,
  .open		= fs_open,
  .read		= fs_read,
  .write		= fs_write,
  .statfs		= fs_statfs,
  .flush		= fs_flush,
  .release	= fs_release,
  .fsync		= fs_fsync,
#ifdef HAVE_SETXATTR
  .setxattr	= fs_setxattr,
  .getxattr	= fs_getxattr,
  .listxattr	= fs_listxattr,
  .removexattr	= fs_removexattr,
#endif
  .lock		= fs_lock,

  .flag_nullpath_ok = 1,
};




int main(int argc, char *argv[]){
  
    int success = 1;
    
    list_t * fuse_args = list_t_new();
    
    char * prospective_root = preparse_opts( &argc, argv, fuse_args );

    success = success && initialize_environment( prospective_root );
    success = success && initialize_redirect( );
    success = success && initialize_regex( );
    success = success && initialize_default_hidden_lists();

    if( success ){
        
        success = fuse_main_real( 
            fuse_args->length,
            fuse_args->data,
            &fs_oper,
            sizeof(fs_oper),
            NULL
        );
        
    }

    return success;
    
}
