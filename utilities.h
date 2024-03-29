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
#include <stdlib.h>

#ifndef UTILITIES_H
#define UTILITIES_H

typedef struct {
  
  int length;
  char ** data;
  int sorted;
  
} list_t;

int    startswith( const char * string, const char * term );
int    endswith( const char * string, const char * term );
char * substring( const char * string, int start, int end );

list_t * list_t_new( );
list_t * list_t_new_from_file( char * path );
list_t * list_t_new_listdir( const char * path );
void     list_t_append( list_t * list, char * string );
void     list_t_append_unique( list_t * list, char * string );
void     list_t_extend( list_t * list, list_t * second );
void     list_t_extend_unique( list_t * list, list_t * second );
int      list_t_contains( list_t * list, char * string );
void     list_t_print( list_t * list );
void     list_t_printf( FILE * file, list_t * list );
void     list_t_sort( list_t * list );
void     list_t_free( list_t * list );


char * add( const char * str1, const char * str2 );
char * cat( char * str1, const char * str2, int do_free );
char * join( list_t * path, char * delim );

/** In prep for implementing a dictionary, this is for 
 * eventual use with attribute, and resolution caching.
 * */
typedef struct {
  
  char * key;
  char * val;
  int    age;
  
} cache_dict_t_node;

typedef struct {
  
  cache_dict_t_node * list;
  int length;
  
} cache_dict_t;


FILE * log_f( const char * title, const char * message );

#endif