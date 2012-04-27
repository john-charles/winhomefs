/*
* WINHOMEFS This is a filesystem use for mouting
* and sharing a windows home directory with a linux
* instance without creating the big mess.
* 
* This file system is written by John-Charles D. Sokolow
* Copyright (C) 2011-1012 John-Charles D. Sokolow <john.charles.sokolow@gmail.com>
* 
* Based heavily on the example fuse file system "fusexmp_fh.c"
* Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
* 
*
* License:  This  program  is  free  software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published by
* the  Free Software Foundation; Version 2 This program is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*/ 

#include <fuse_opt.h>
#include "utilities.h"

#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

char * preparse_opts( int * argc, char * argv[], list_t * fuse_args );
struct fuse_args * parse( int argc, char * argv[] );

#endif