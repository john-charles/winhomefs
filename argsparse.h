#include <fuse_opt.h>
#include "utilities.h"

#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

char * preparse_opts( int * argc, char * argv[], list_t * fuse_args );
struct fuse_args * parse( int argc, char * argv[] );

#endif