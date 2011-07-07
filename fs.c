#include "fs.h"
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

extern const char * hello_str;
extern const char * hello_path;

static int fs_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    
    char * real_path = resolve( path );
    
    stat( real_path, stbuf );
    
    free( real_path );

    return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
    
    char * real_path = resolve( path );
    
    struct dirent * entry;
    
    DIR * dp = opendir( real_path );
    
    while( entry = readdir( dp ) ){
      filler( buf, entry->d_name, NULL, 0 );
    }

//     if(strcmp(path, "/") != 0) return -ENOENT;
// 
//     filler(buf, ".", NULL, 0);
//     filler(buf, "..", NULL, 0);
//     filler(buf, hello_path + 1, NULL, 0);

    return 0;
}


