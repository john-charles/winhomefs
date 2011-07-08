#include <stdio.h>

#include "utilities.h"

extern list_t * hidden_list_home;
extern list_t * hidden_list_user;

static int is_hidden( char * path ){
 
  char * hidden_p = cat( path, "/.hidden", 0 );
  FILE * hidden_f = fopen( hidden_p, "rb" );
  
  if( hidden_f ){
    
    
    
  }
  
}