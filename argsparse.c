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
#include <fuse_opt.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "utilities.h"


char * preparse_opts( int * argc, char * argv[], list_t * fuse_args ){
  
  if ( *argc < 3 ){
    puts("Sorry invalid number of arguments...");
    puts("Expecting exec_name /path/to/source/directory /path/to/mountpoint");
    exit(1);
  }
  
  char * root = 0;
  
  int i;
  
  list_t_append( fuse_args, argv[0] );
  
  for( i = 1; i < *argc; i++ ){
    
    if( strcmp( argv[i], "-o" ) == 0 ){
      
      list_t_append( fuse_args, "-o" );
      list_t_append( fuse_args, argv[i+1] );
       
      i++;
      /* skip this option */
      
    } else if( argv[i][0] == '-' ){
      list_t_append( fuse_args, argv[i] );
    } else if( root == 0 ){
      root = cat( argv[i], "", 0 );
    } else {
      list_t_append( fuse_args, argv[i] );
    }
  }
   
  list_t_print( fuse_args );
    
  return root;
  
}
      
      

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


struct fuse_args * parse( int argc, char * argv[] ){
  
  
  
  
  return 0;
  
}