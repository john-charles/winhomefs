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

char * root_hidden[] = {
  "All Users","Default","Default User","Public",".",".."};
int sizeof_root_hidden = sizeof( root_hidden );

char * home_hidden[] = {
  "Application Data","Cookies","Local Settings","NetHood",
  "NTUSER.*","ntuser.*","PrintHood","Recent","SendTo","Start Menu"};
int sizeof_home_hidden = sizeof( home_hidden );

char * user_hidden[] = {
  "Desktop.ini","desktop.ini","Thumbs.db"};
int sizeof_user_hidden = sizeof( user_hidden );

char * vista_home_hidden[] = {
  "AppData","My Documents"};
int sizeof_vista_home_hidden = sizeof( vista_home_hidden );





int compare_strings( const void * a, const void * b ){
  
  const char ** sa = ( const char ** ) a;
  const char ** sb = ( const char ** ) b;
  //printf("compare_strings(%s, %s)\n", *sa, *sb );
  return strcmp( *sb, *sa );
   
}

struct hidden_mapping {
  char * directory_path;
  char * hidden_list[];
};

struct hidden_mapping * hidden;


char * bsearch_strings( char * term, char * list[], int sizeof_list ){
  
  //printf("sizeof(list)=%i\n", sizeof_root_hidden );
  
  //printf("bsearch_strings term=%s\n", term);
  char * term_array[] = { term };
  char ** result;
  //puts("About to do real search...");
  int count = sizeof_list / sizeof( char * );
  //printf("bsearch count is %i\n", count );
  result = bsearch( term_array, list, count, sizeof( char * ), compare_strings );
  //puts("searched...");
  
  if( result ){
    
    return * result;
    
  }
  
  return 0;
  
}

void qsort_strings( char * list[], int sizeof_list ){
  
  int count = sizeof_list / sizeof( char * );
  qsort( list, count, sizeof( char * ), compare_strings );
  
}


int initialize_hidden( ){
  
  qsort_strings( home_hidden, sizeof_home_hidden );
  qsort_strings( root_hidden, sizeof_root_hidden );
  qsort_strings( user_hidden, sizeof_user_hidden );
  qsort_strings( vista_home_hidden, sizeof_vista_home_hidden );
  
}


static struct environment {
  
  char * real_root;
  char * winversion;
  char * d_redirect;
  
} environment;



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
      environment.d_redirect = "AppData/Linux";
    
      
    } else {
      
      puts("Found \"Documents and Settings\" folder, assuming windows 2000 and above!");
      environment.winversion = "win2k";
      environment.d_redirect = "Linux Data";
    
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
  free( stbuf );stbuf = 0;
  return 0;
  
}

regex_t user_home_dir;
regex_t user_dot_folder;
regex_t user_doc_subdir;
regex_t user_mdocs_dir;


static int initialize_regular_expressions( void ){
  
  int failure;
  
  failure = regcomp( &user_home_dir, "^/[^/]+$", REG_EXTENDED );
  
  if( failure ){
    
    puts("Failed to compile homedir expression!");
    return failure;
    
  }
  
  failure = regcomp( &user_dot_folder, "(^/[^/]+)/(\\..*$)", REG_EXTENDED );
  
  if( failure ){
    
    puts("Failed to compile user_dot_folder regex...");
    return failure;
    
  }
    
  failure = regcomp( &user_doc_subdir, "(^/.*)/(My Documents/My [Music|Pictures|Videos]+)/(.*$)", REG_EXTENDED );
  
  if( failure ){
    
    puts("Failed to compile user_documentsf reges...");
    return failure;
    
  }
  
  failure = regcomp( &user_mdocs_dir, "(^/.*)/(My Documents)/(.*$)", REG_EXTENDED );
  
  if( failure ) {
    
    puts("Failed to compile user_docs_dir regex...");
    return failure;
    
  }
  
}




int substring( char * destination, const char * source, int start, int end ){
  
  int i = 0;
  
  for( start; start < end; start++, i++ ){
    
    destination[i] = source[ start ];
    
  }
  
  destination[i] = '\x00';//Make sure our new string is a proper string.
}

typedef struct {
  
  char * path;
  char ** hidden;
  int hidden_size;
  
} resolve_t;

resolve_t * resolve( const char * path, int hidden ){
  
  puts("in resolve.");
  
  resolve_t * res = (resolve_t*)malloc( sizeof( resolve_t ) );
  
  puts("allocated resolve_t memory...");
  
  char * result = 0;
  int size = 0;
  
  regmatch_t match[3];
  
  int failure = 1;
  
  memset( res, 0, sizeof( resolve_t ) );
  
  if( strcmp( path, "/" ) == 0 ){
    
    res->path = (char*)malloc( strlen( environment.real_root ) );    
    
    sprintf( res->path, "%s", environment.real_root );
    
    if( hidden ){
      
      res->hidden = root_hidden;
      res->hidden_size = sizeof_root_hidden;
      
    }
    
    return res;
    
  }
  
  failure = regexec( &user_home_dir, path, 1, match, 0 );
  
//   if( !failure && hidden ){
//       
//       hidden = home_hidden;
//       *hidden_size = sizeof_home_hidden;
//   }
  
  memset( match, 0, sizeof( match ) );   
  
  failure = regexec( &user_dot_folder, path, 3, match, 0 );
  
  //puts("evaluated regular expression...");
  
  if( !failure ){
    
    //printf("Matched a regular expression...");puts("");
    //printf("path is %s", path );puts("");
    //printf("match[0].rm_eo = %i", match[0].rm_eo );puts("");
    //printf("match[0].rm_so = %i", match[0].rm_so );puts("");
    
    size = match[1].rm_eo - match[1].rm_so;    
    char * base = 0;
    base = realloc( base, size );
    
    substring( base, path, match[1].rm_so, match[1].rm_eo );
    
    size = match[2].rm_eo - match[2].rm_so;
    char * end = 0;
    end = realloc( end, size );
    
    substring( end, path, match[2].rm_so, match[2].rm_eo );
    
    size = strlen( environment.real_root ) + strlen( base )
     + strlen( environment.d_redirect )    + strlen( end ) + 3;
    
    result = realloc( result, size );
    /* NOTE: base starts with a / so the second string does 
     * not need to be followed by one */
    sprintf( result, "%s%s/%s/%s", environment.real_root,
             base, environment.d_redirect, end );
    
    free(base);base=0;
    free(end);end=0;
    
//     if( hidden ){
//       
//       hidden = user_hidden;
//       *hidden_size = sizeof_user_hidden;
//     }
    
    
    res->path = result;
    
    if( hidden ){
      res->hidden = user_hidden;
      res->hidden_size = sizeof_user_hidden;
    }    
    
    return res;
    
  }
  
//   if( (int)hidden == 1 ){
//     
//     hidden = user_hidden;
//     *hidden_size = sizeof_user_hidden;
//   }
  
  size = strlen( environment.real_root ) + strlen( path );
  result = realloc( result, size );
  sprintf( result, "%s%s", environment.real_root, path );
  
  
  res->path = result;
  
  if( hidden ){
    res->hidden = user_hidden;
    res->hidden_size = sizeof_user_hidden;
  }    
  
  return res;
  
}
  
static int hello_getattr(const char *path, struct stat *stbuf)
{
    
    memset(stbuf, 0, sizeof(struct stat));
    
    resolve_t * real_path = resolve( path, 0 );
    
    stat( real_path->path, stbuf );
    
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR || 0755;
        stbuf->st_uid  = 0;
        stbuf->st_gid  = 0;
    }
    
    free( real_path );
    
    return 0;
}

int valid_line( char * line ){
  
  return line[0] != '\x00' && line[0] != '\n' && line[0] != ' ';
  
}

char * copystr( char * str, int begin, int end ){
  int size =  end - begin;
  if( size > 0 ){
    char * dest = (char*)malloc( size );
    substring( dest, str, begin, end );
    return dest;
  }
  return 0;
}
  

char ** append( char ** list, int * list_size, char * string, int copy ){
  
  if( *list_size == 0 ){
    list = realloc( list, sizeof( char ** ) );
    *list_size = 1;
  } else {
    list = realloc( list, sizeof( char ** ) * ++(*list_size) );
  }
  
  if( copy ){
    
    list[ *list_size ] = (char*)malloc( strlen( string ) );
    strcpy( list[*list_size], string );
    
  } else {
    
    list[*list_size ] = string;
    
  }
    
  return list;
  
}
  
  

typedef struct {
  char ** list;
  int     leng;
} hidden_list_t;

hidden_list_t * read_file( const char * path ){
  
  FILE * file = fopen( path, "rb" );  
  
  int    size = 20;
  char * buff = (char*)malloc( 20 );
  
  int    read = 0;
  char * line = 0;
    
  hidden_list_t * hidden = (hidden_list_t*)malloc(sizeof(hidden_list_t));
  hidden->list = 0;
  hidden->leng = -1;
  
  if( file ){    
    read = getline( &buff, &size, file );  
    while( read != -1  ){      
      if( valid_line( buff ) ){        
        
        hidden->list = append( hidden->list, &hidden->leng, copystr( buff, 0, strlen( buff ) -1 ), 0 );
        
        //printf("list[%i] = %s\n", hidden->leng, hidden->list[hidden->leng] );
        
      }
      
      read = getline( &buff, &size, file ); 
      
    }
    
    hidden->leng++;
    
  } 
  
  fclose( file );
  
  return hidden;    
}

const char ** load_hidden( const resolve_t * info ){
  
    char * hidden_name = ".hidden";
    char * hidden_path = (char*)malloc( strlen( info->path ) + strlen( hidden_name ) + 1 );
    
    sprintf( hidden_path, "%s/%s", info->path, hidden_name );
    puts( hidden_path );
    
    hidden_list_t * final_list = read_file( hidden_path );
    
    int i = 0;
    char * temp = 0;
    for( i = 0; i < info->hidden_size; i++ ){
      
      final_list->list = append( final_list->list, &final_list->leng, info->hidden[i], 1 );
      
    }
      
      
    
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



int initialize_components( char * root ){
  
  int failure;
  
  failure = initialize_hidden( );
        
  failure = initialize_environment( root );
  
  if( failure ){
    
    puts("Failed to properly initialize environment...");
    return failure;
    
  }
  
  failure = initialize_regular_expressions( );
  
  if( failure ){
    
    puts("Failed to compile necessary regular expression patterns...");
    return failure;
    
  }
  
  return 0;
    
}

int main(int argc, char *argv[]){
  
  if( argc > 1 ){
    
    if( initialize_components( argv[1] ) != 0 ){
      
      return -1;
      
    }
        
  } else {
    
    puts("Please specify a user dir to mount on...");
    return -1;
    
  }
  
  resolve_t * res = resolve( "/", 1 );
  puts( res->path );
  load_hidden( res );
  
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