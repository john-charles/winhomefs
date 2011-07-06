#include <fuse_opt.h>

#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

char * preparse_opts( int * argc, char * argv[] );
struct fuse_args parse( struct fuse_args * args, int argc, char * argv[] );

#endif