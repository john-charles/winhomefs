
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