/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2005  Miklos Szeredi <miklos@szeredi.hu>

   Changed: Added fuse_opt.h support to show
            cmdline-option passing (ingenious) / code clean (rigid)


    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.
*/

#define FUSE_USE_VERSION 25

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <regex.h>
#include <fuse.h>
#include <fuse_opt.h>

#include "fs.c"
#include "hidden.h"
#include "argsparse.h"
#include "utilities.h"
#include "resolve.h"
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


   
const char *hello_str = "Hello World!\n";
const char *hello_path = "/hello";

// static int hello_open(const char *path, struct fuse_file_info *fi)
// {
//     if(strcmp(path, hello_path) != 0)
//         return -ENOENT;
// 
//     if((fi->flags & 3) != O_RDONLY)
//         return -EACCES;
// 
//     return 0;
// }

// static int hello_read(const char *path, char *buf, size_t size, off_t offset,
//                       struct fuse_file_info *fi)
// {
//     size_t len;
//     (void) fi;
//     if(strcmp(path, hello_path) != 0)
//         return -ENOENT;
// 
//     len = strlen(hello_str);
//     if (offset < len) {
//         if (offset + size > len)
//             size = len - offset;
//         memcpy(buf, hello_str + offset, size);
//     } else
//         size = 0;
// 
//     return size;
// }

// .getattr        = xmp_getattr,
// .access         = xmp_access,
// .readlink       = xmp_readlink,
// .readdir        = xmp_readdir,
// .mknod          = xmp_mknod,
// .mkdir          = xmp_mkdir,
// .symlink        = xmp_symlink,
// .unlink         = xmp_unlink,
// .rmdir          = xmp_rmdir,
// .rename         = xmp_rename,
// .link           = xmp_link,
// .chmod          = xmp_chmod,
// .chown          = xmp_chown,
// .truncate       = xmp_truncate,
// .utimens        = xmp_utimens,
// .open           = xmp_open,
// .read           = xmp_read,
// .write          = xmp_write,
// .statfs         = xmp_statfs,
// .release        = xmp_release,
// .fsync          = xmp_fsync,
// #ifdef HAVE_SETXATTR
// .setxattr       = xmp_setxattr,
// .getxattr       = xmp_getxattr,
// .listxattr      = xmp_listxattr,
// .removexattr    = xmp_removexattr,
// #endif


static struct fuse_operations fs_operations = {
    .access    = fs_access,
    .chmod     = fs_chmod,
    .chown     = fs_chown,
    .fsync     = fs_fsync,    
    .getattr   = fs_getattr,
    //.getxattr... not implemented...
    .link      = fs_link,
    .mkdir     = fs_mkdir,
    .mknod     = fs_mknod,
    .open      = fs_open,
    .read      = fs_read,
    //.listxattr=
    .readdir   = fs_readdir,
    .readlink  = fs_readlink,
    .release   = fs_release,
    .rename    = fs_rename,
    .rmdir     = fs_rmdir,
    
    .write     = fs_write,
};
  
  

int main(int argc, char *argv[])
{
  
  int success = 1;  
  char * prospective_root = preparse_opts( &argc, argv );
   
  success = success && initialize_environment( prospective_root );
  success = success && initialize_redirect( );
  success = success && initialize_regex( );
  success = success && initialize_default_hidden_lists();
  
  if( success ){
    
    
//     puts( resolve("/.hidden") );
//     puts( resolve("/My Hobbide") );
//     puts( resolve("/Dogs & Stuff") );
//     puts( resolve("/My Documents/My Videos/HQ DVD RIP") );
//     puts( resolve("/My Documents/My Music") );
//     puts( resolve("/My Documents/My Music/Collection") );
//     puts( resolve("/My Documents/My Pictures") );
//     puts( resolve("/My Documents/My Pictures/2011/March/ProfilePictures/001.jpg") );
//     
//     char * buff = (char*)malloc(4096);
//     memset( buff, 0, 4096 );
//     char * p = resolve("/hello");
//     printf("p == %s\n", p );
//     FILE * hello = fopen(p,"rb");
//     printf("errno == %i\n", errno );
//     int read = pread( hello, buff, 4096, 0 );
//     printf("read from errno == %i\n", errno );
//     printf("read from hello %i bytes\n", read );
//     printf("read from hello %s\n", buff );
    
//     list_t * hidden = get_hidden_list( resolve("/") );
//     list_t_print( hidden );
//     
//     puts("");
//     printf("AppData list_result == %i\n", list_t_contains( hidden, "AppData" ) );
//     
//     list_t_free( hidden );
      
    /** Begin fuse initialization **/
    /* but skip for now */
//     return 0;
    
    log_f("test.log.txt","This is a log entry!");
    
  
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    parse( &args, argc, argv );
    success = fuse_main(args.argc, args.argv, &fs_operations);
    
    /** free arguments */
    fuse_opt_free_args(&args);
    
  }

  return success;
}