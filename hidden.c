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