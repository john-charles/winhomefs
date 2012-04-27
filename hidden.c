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

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "utilities.h"

extern char * root_path;

extern list_t * hidden_list_home;
extern list_t * hidden_list_user;

list_t * get_hidden_list( const char * path ){
 
  char   * hidden_p = cat((char*) path, "/.hidden", 0 );
  FILE   * hidden_f = fopen( hidden_p, "rb" );  
  list_t * hidden_l = list_t_new();
  
  if( strcmp( path, root_path ) == 0 ){
    
    list_t_extend( hidden_l, hidden_list_home );
    
  } 
  
  list_t_extend( hidden_l, hidden_list_user );
    
  
  
  if( hidden_f ){
    
    size_t size = 100;
    char * buff = (char*)malloc(100);
    int    read = getline( &buff, &size, hidden_f );
    
    while( read > 0 ){
      
      list_t_append( hidden_l, substring( buff, 0, strlen( buff ) -1 ));
      read = getline( &buff, &size, hidden_f );
      
    }
    
    fclose( hidden_f );
    free( buff );
    
  }
  
  free( hidden_p );
  list_t_sort( hidden_l );
  
  return hidden_l;
  
}