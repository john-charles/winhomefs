/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2005  Miklos Szeredi <miklos@szeredi.hu>

   Changed: Added fuse_opt.h support to show
            cmdline-option passing (ingenious) / code clean (rigid)


    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.
*/

#define FUSE_USE_VERSION 25
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <dirent.h>
#include <regex.h>
#include <fuse.h>
#include <fuse_opt.h>
#include <sys/stat.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

/** options for fuse_opt.h */
struct options {
   
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
      // #define FUSE_OPT_KEY(templ, key) { templ, -1U, key }
      FUSE_OPT_KEY("-V",             KEY_VERSION),
      FUSE_OPT_KEY("--version",      KEY_VERSION),
      FUSE_OPT_KEY("-h",             KEY_HELP),
      FUSE_OPT_KEY("--help",         KEY_HELP),
      FUSE_OPT_END
};

char * home_hidden[] = {
  "Application Data","Cookies","Local Settings","NetHood",
  "NTUSER.*","ntuser.*","PrintHood","Recent","SendTo","Start Menu"  
};

char * vista_home_hidden[] = {
  "AppData","My Documents"};
  
char * user_hidden[] = {
  "Desktop.ini","desktop.ini","Thumbs.db"
};

struct hidden_mapping {
  char * directory_path;
  char * hidden_list[];
};

struct hidden_mapping * hidden;

static struct environment {
  
  char * real_root;
  char * winversion;
  
} environment;

#define OUT_OF_MEMORY -100

static char * join( const char * first, const char * second ){
  
  char * s = malloc(snprintf(NULL, 0, "%s/%s", first, second) + 1 );
  sprintf(s, "%s/%s", first, second);
  return s;
  
}

static int initialize_environment( const char * root_directory ){
  
  printf("Validating target directory %s\n", root_directory );
  
  struct stat * stbuf = ( struct stat * ) malloc( sizeof( struct stat ) );
  
  if( stbuf == 0 ){
    // Were not getting the memory we need....?
    puts("Out of memory!");
    return ENOMEM;
  }
  
  // clear the stat buff...
  memset( stbuf, 0, sizeof( struct stat ) );
  
  int success = stat( root_directory, stbuf );
  
  if( success == -1 ){
    
    printf("The directory %s does not seem to exist, or is inaccessable!\n", root_directory );
    return ENOENT;
  }
  
  if( !S_ISDIR( stbuf->st_mode )){
    
    printf("The directory %s is not a directory, please provied a valid directory.\n", root_directory );
    return ENOTDIR; /* Not a directory */
    
  }
  
  DIR * dir = opendir( root_directory );
  
  if( dir == 0 ){
    
    printf("Could not open directory %s for reading, please ensure read access!\n", root_directory );
    return EIO;
    
  }
  
  struct dirent * entry = readdir( dir );
  
  int users_folder_found = 0; // Does a vista User dir exists...
  int docsn_folder_found = 0; // Does a win2k Documents and Settings folder exist...
  
  while( entry != 0 ){
    
    if( strcmp( entry->d_name, "Users" ) == 0 ){
      users_folder_found = 1;
    }
    if( strcmp( entry->d_name, "Documents and Settings" ) == 0 ){
      docsn_folder_found = 1;
    }
    entry = readdir( dir );
  }
  
  closedir( dir );
  
  if( users_folder_found || docsn_folder_found ){
    
    if( users_folder_found ){
      
      puts("Found \"Users\" folder, asuming windows version Vista and above!");
      environment.winversion = "vista";
    
      
    } else {
      
      puts("Found \"Documents and Settings\" folder, assuming windows 2000 and above!");
      environment.winversion = "win2k";
    
    }
    
  } else {
    
    puts("Could not find a profile directory.");
    return ENOENT;
    
  }
  
  
  char * user_directory;
    
  if( strcmp( environment.winversion, "win2k" ) == 0 ){
    
    user_directory = join( root_directory, "Documents and Settings" );
       
  } else {
    
    user_directory = join( root_directory, "Users" );
    
  }
  
  // clear the stat buff...
  memset( stbuf, 0, sizeof( struct stat ) );
  
  if( stat( user_directory, stbuf ) == 0 ){
    
    if( S_ISDIR( stbuf->st_mode ) ){
      
      printf("Valideated %s\n", user_directory );
        
    } else {
      
      puts("Prifle directory was found, but was not of type directory!");
      return ENOTDIR;
      
    }
    
  } else {
    
    printf("Could not stat profile directory %s", user_directory );
    return EIO;
    
  }
  
  environment.real_root = user_directory;
  
  free( stbuf );
  
}


regex_t user_dot_folder;
regex_t user_documentsf;

static void initialize_regular_expressions( void ){
  
  regcomp( &user_dot_folder, "^/.*/\\..*$", 0 );
  regcomp( &user_documentsf, "^/.*/My .*/", 0 );
  
}

char * resolve( const char * path ){
  
  

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





int main(int argc, char *argv[]){
  
    
  if( argc > 1 ){
    initialize_environment( argv[1] );
    initialize_regular_expressions( );
  }
  
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