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

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;

    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;

    len = strlen(hello_str);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, hello_str + offset, size);
    } else
        size = 0;

    return size;
}

static struct fuse_operations fs_operations = {
    .getattr   = fs_getattr,
    .readdir   = hello_readdir,
    .open      = hello_open,
    .read      = hello_read,
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
    
    
  
    puts( resolve("/My Hobbide") );
    puts( resolve("/Dogs & Stuff") );
    puts( resolve("/My Documents/My Videos/HQ DVD RIP") );
    puts( resolve("/My Documents/My Music") );
    puts( resolve("/My Documents/My Music/Collection") );
    puts( resolve("/My Documents/My Pictures") );
    puts( resolve("/My Documents/My Pictures/2011/March/ProfilePictures/001.jpg") );
    
    /** Begin fuse initialization **/
    /* but skip for now */
    //return 0;
    
    log_f("test.log.txt","This is a log entry!");
    
  
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    parse( &args, argc, argv );
    success = fuse_main(args.argc, args.argv, &fs_operations);
    
    /** free arguments */
    fuse_opt_free_args(&args);
    
  }

  return success;
}