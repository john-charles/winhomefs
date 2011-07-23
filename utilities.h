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