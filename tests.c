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
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 * 
 */
#include "utilities.h"

void test_list_t( ){
  
  list_t * my_list = list_t_new();
  
  list_t_append( my_list, "Z" );
  list_t_append( my_list, "B" );
  list_t_append( my_list, "A" );
  list_t_append( my_list, "J" );
  list_t_append( my_list, "L" );
  
  list_t_print( my_list );
  
  list_t_sort( my_list );
  puts("");
  list_t_print( my_list );
  puts("");
  
  list_t_append_unique( my_list, "Z" );
  list_t_print( my_list );puts("");
  
  list_t_free( my_list );
  
  list_t * hidden = list_t_new_from_file("/mnt/Personal/Users/.hidden");
  list_t_sort( hidden );
  list_t_print( hidden );
  list_t_free( hidden );
  
  
}