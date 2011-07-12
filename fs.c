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

static int fs_getattr(const char *path, struct stat *stbuf)
{   
  
  
    FILE * log_file = log_f("fs_getattr.txt","");
    
    fprintf(log_file,"getattr called:\n");
    fprintf(log_file,"    call path is %s\n", path );
    
    
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    
    char * real_path = resolve( path );
    fprintf(log_file,"    resolve path is %s\n", real_path );
    
    
    
    
    
    int srtn = stat( real_path, stbuf );
    fprintf( log_file, "   stat said %i\n", srtn );
    if (srtn == 0 ){
      
      res = 0;
      
    } else {
      
      res = -errno;
      
    }
    
    fprintf( log_file, "    return code is %i\n", res );
    
    fflush( log_file );
    fclose( log_file );
    
    free( real_path );
    
    return res;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi)
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

    list_t_free( hidden );
    
    return 0;
}


static int fs_open(const char *path, struct fuse_file_info *fi)
{
  FILE * log_file = log_f("fs_open","");
  
  char * real_path = resolve( path );
  fi->fh = open( real_path, fi->flags );
  fprintf( log_file, "real path is %s\n", real_path );
  fprintf( log_file, "fi->fh == %i\n", (int) fi->fh );
  close( log_file );
  if( fi->fh == -1 ){    
    return -errno;    
  }
      
  return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  FILE * log_file = log_f("hello_read","");
  fprintf( log_file, "fi->fh == %i\n",(int) fi->fh );
  int return_code = 0;
  
  if( fi->fh == -1 ){
    
    return_code = -EBADF;
    fprintf( log_file, "file_handle == -1\n");
        
  } else {
    
    memset( buf, 0, size );
    
    fprintf( log_file, "size is %i offset is %i\n", (int) size, (int) offset );
    return_code =  pread( fi->fh, buf, size, offset );
    fprintf( log_file, "buff contains %s\n", buf );
    fprintf( log_file, "called pread, return code is %i\n", return_code );
  }
  
  return return_code;
    
}


