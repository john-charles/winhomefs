#include "fs.h"
#include "hidden.h"
#include "resolve.h"
#include "utilities.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <regex.h>
#include <fuse.h>
#include <dirent.h>
#include <fuse_opt.h>
#include <sys/stat.h>

extern list_t * hidden_list_home;
extern list_t * hidden_list_user;

extern char * winredirect;

extern const char * hello_str;
extern const char * hello_path;

static int fs_access(const char *path, int mask)
{
        int res;
        char * real_path = resolve( path );
        res = access(real_path, mask);
        if (res == -1)
                return -errno;

        return 0;
}

static int fs_chmod(const char *path, mode_t mode)
{
  int res;
  char * real_path = resolve(path);
  res = chmod(real_path, mode);
  if (res == -1)
    return -errno;

  return 0;
}

static int fs_chown(const char *path, uid_t uid, gid_t gid)
{
  int res;
  char * real_path = resolve( path );
  res = lchown(real_path, uid, gid);
  if (res == -1)
    return -errno;

  return 0;
}

static int fs_fsync(const char *path, int isdatasync,  struct fuse_file_info *fi)
{
  /* Just a stub.  This method is optional and can safely be left
      unimplemented */

  (void) path;
  (void) isdatasync;
  (void) fi;
  return 0;
}


static int fs_getattr(const char *path, struct stat *stbuf)
{   
  
  
  
        
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    
    char * real_path = resolve( path );
       
    
    int srtn = stat( real_path, stbuf );
    
    if (srtn == 0 ){
      
      res = 0;
      
    } else {
      
      res = -errno;
      
    }
    
        
    free( real_path );
    
    return res;
}

static int fs_link(const char *from, const char *to)
{
    int res;
    char * real_from = resolve( from );
    char * real_to   = resolve( to );
    res = link(real_from, real_to);
    free( real_from );
    free( real_to );
    if (res == -1)
        return -errno;

    return 0;
}

static int fs_mkdir(const char *path, mode_t mode)
{
    int res;
    char * real_path = resolve( path );
    printf("fs.c: fs_mkdir: path is %s\n",path);
    printf("fs.c: fs_mkdir: real_path is %s\n", real_path );
    printf("fs.c: fs_mkdir: mode is %i\n", mode );
    
    res = mkdir( real_path, 0700);
    free( real_path );
    if (res == -1){
      printf("fs.c: fs_mkdir: an error ocurred... errno is %i\n", errno );
      printf("fs.c: fs_mkdir: checking access on / %i\n", access( "/mnt/Personal/Users/john-charles", W_OK ) );
      printf("fs.c: fs_mkdir: checking access on / %i\n", access( "/mnt/Personal/Users/john-charles", F_OK ) );
      return -errno;
    }

    return 0;
}

static int fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
  int res;
  char * real_path = resolve( path );
  /* On Linux this could just be 'mknod(path, mode, rdev)' but this
  is more portable */
  if (S_ISREG(mode)) {
    res = open(real_path, O_CREAT | O_EXCL | O_WRONLY, mode);
    if (res >= 0){
      res = close(res);
    }
  } else if (S_ISFIFO(mode)){
    res = mkfifo(real_path, mode);
  } else {
    res = mknod(real_path, mode, rdev);
  }
  
  if (res == -1){
    return -errno;
  }
  return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
  printf("fs.c: fs_open: path is %s\n", path );
  char * real_path = resolve( path );
  
  printf("fs.c: fs_open: resolved to %s\n", real_path );
  printf("fs.c: fs_open: fi->flags == %i\n", fi->flags );
  fi->fh = open( real_path, fi->flags ); 
  free( real_path );
  if( fi->fh == -1 ){
    printf("fs.c: fs_open: failed to open file, errno == %i\n", errno );
    return -errno;    
  } else {
    
  }
  
  return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  puts("fs.c: hello_read: read called on previously opened file...");
  printf("fs.c: hello_read: fi->flages is %i\n",(int)fi->flags);
  int return_code = 0;
  char * real_path = resolve( path );
  memset( buf, 0, size );
  int fd = open( real_path, fi->flags );
  if( fd == -1 ){
    return_code = -errno;
  } else {
    FILE * open_file = fdopen( fd, "rb" );
    fseek( open_file, offset, SEEK_SET );
    return_code =  fread( buf, 1, size, open_file );
    if( return_code == -1 ){
      return_code = -errno;
    }
  }
  
  
  return return_code;
    
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
    
    char * real_path = resolve( path );
    
    list_t * hidden = get_hidden_list( real_path );
    
    struct dirent * entry;
    
    DIR * dp = opendir( real_path );
    
    if( strcmp( path, "/" ) == 0 ){
      
      while( entry = readdir( dp ) ){
        
        if( !list_t_contains( hidden, entry->d_name ) && entry->d_name[0] != '.' ){
          filler( buf, entry->d_name, NULL, 0 );
        }
      }
    
      closedir( dp );     
      dp = opendir( winredirect );
      
      while( entry = readdir( dp ) ){
        
        filler( buf, entry->d_name, NULL, 0 );
                
      }
                 
    } else {
      
      while( entry = readdir( dp ) ){
        
        if( !list_t_contains( hidden, entry->d_name ) ){
          filler( buf, entry->d_name, NULL, 0 );
        }
      }
      
    }
    
    closedir( dp );
  free(real_path);
    list_t_free( hidden );
    
    return 0;
}

static int fs_readlink(const char *path, char *buf, size_t size)
{
  int res;
  char * real_path = resolve( path );
  res = readlink(real_path, buf, size - 1);
  if (res == -1)
    return -errno;

  buf[res] = '\0';
  return 0;
}


static int fs_release(const char *path, struct fuse_file_info *fi)
{
    /* Just a stub.  This method is optional and can safely be left
       unimplemented */
    int rtn = 0;
    if( fi->fh > 0 ){
      rtn = close( fi->fh );
      if( rtn == -1 && errno == EINTR ){
        rtn = fs_release( path, fi );
      } else if( rtn == -1 ){
        rtn = -errno;
      }
      
      fi->fh = -1;
      
    } else {
      rtn = -EBADF;
    }
        
    return rtn;
}

static int fs_rename(const char *from, const char *to)
{
  int res;
  char * real_from = resolve( from );
  char * real_to   = resolve( to  );

  res = rename(real_from, real_to);
  if (res == -1){
    res = -errno;
  }
  free( real_from );
  free( real_to   );
  return res;
}

static int fs_rmdir(const char *path){
  
  if( strcmp( path, "/Music" ) == 0 || strcmp( path, "/Pictures" ) == 0 || strcmp( path, "/Videos" ) == 0 || strcmp( path, "/Documents" ) == 0 ){
    
    return -EACCES;
    
  } else {
    
    char * real_path = resolve( path );
    int    ret = rmdir( real_path );
    
    free( real_path );
    
    if( ret == -1 ){
      return -errno;
    }
  }
  
  return 0;
//     int res;
// 
//     res = rmdir(path);
//     if (res == -1)
//         return -errno;
// 
//     return 0;
}


static int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
  
  puts("fs.c: fs_write:...");
  
  if( fi->fh == -1 ){
    return -EBADF;
  } else {
    
    FILE * file = fdopen( fi->fh, "wb" );
    fseek( file, 0, SEEK_SET );
    int retn = fwrite( buf, 1, size, file );
    
    if( retn != size ){
      return -EIO;
    } else {
      fflush( file );
      return retn;
    }
  } 
  
}



