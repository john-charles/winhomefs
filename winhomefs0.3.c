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

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

/** options for fuse_opt.h */
struct options {
   char* foo;
   char* bar;
   int baz;
   double quux;
}options;

/** macro to define options */
#define HELLOFS_OPT_KEY(t, p, v) { t, offsetof(struct options, p), v }

/** keys for FUSE_OPT_ options */
enum
{
   KEY_VERSION,
   KEY_HELP,
};

static struct fuse_opt hello_opts[] =
{      
  FUSE_OPT_KEY("-v",             KEY_VERSION),
  FUSE_OPT_KEY("--version",      KEY_VERSION),
  FUSE_OPT_KEY("-h",             KEY_HELP),
  FUSE_OPT_KEY("--help",         KEY_HELP),
  FUSE_OPT_END
};

#include "utilities.h"
#include "initialization.h"

/** Global variables **/

char * root_path;
char * winversion;
char * winredirect;

  
list_t * hidden_list_root = 0;
list_t * hidden_list_home = 0;
list_t * hidden_list_user = 0;

regex_t regex_user_home_dir;
regex_t regex_user_dot_folder;
regex_t regex_user_doc_subdir;
regex_t regex_user_mdocs_dir;


static int hello_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR || 0755;
        stbuf->st_nlink = 2;
    }
    else if(strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG || 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    }
    else
        res = -ENOENT;

    return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    if(strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, hello_path + 1, NULL, 0);

    return 0;
}

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
        if(options.foo || options.bar || options.baz){
         printf("fooooo bar !!\n");
         printf("foo: \"%s\" bar: \"%s\" baz: \"%d\" quux: \"%f\"\n",
            options.foo, options.bar, options.baz, options.quux);
        }
        else

        memcpy(buf, hello_str + offset, size);
    } else
        size = 0;

    return size;
}


/** This tells FUSE how to do every operation */
static struct fuse_operations hello_oper = {
    .getattr   = hello_getattr,
    .readdir   = hello_readdir,
    .open   = hello_open,
    .read   = hello_read,
};



int main(int argc, char *argv[])
{
  
//   char * map_dir = 0;
//   int i;
//   for (i = 1; (i < argc) && (argv[i][0] == '-'); i++){
//     if (argv[i][1] == 'o'){
//       i++;
//       // -o takes a parameter; need to
//       // skip it too.  This doesn't
//       // handle "squashed" parameters
//     } else if ((argc - i) != 2){
//       puts("Error parsing command arguments...");
//       return 0;
//     } else {
//       
//       
//     
//     map_dir = argv[i];
//     argv[i] = argv[i + 1];
//     argc--;
//     
//   }
//   
//   for(i = 0; i < argc; i++ ){
//     puts( argv[i] );
//   }
  int success = 1;
  
  success = success && initialize_environment( "/mnt/Personal" );
  success = success && initialize_regex( );
  success = success && initialize_default_hidden_lists();
  
  printf("success = %i\n", success );
  
  /** Begin fuse initialization **/
  /* but skip for now */
  return 0;
  
  
  int ret;
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);



  /* clear structure that holds our options */
  memset(&options, 0, sizeof(struct options));

  if (fuse_opt_parse(&args, &options, hello_opts, NULL) == -1)
    /** error parsing options */
    return -1;


  ret = fuse_main(args.argc, args.argv, &hello_oper);

  if (ret) printf("\n");

  /** free arguments */
  fuse_opt_free_args(&args);

   return ret;
}