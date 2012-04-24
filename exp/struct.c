
#include <stdlib.h>

struct share_entry {
  
  char * name;
  char * list[];
  
};

struct share_entry ** entries = 0;

int main( void ){
  
  //struct share_entry * entries = 0;
  
  entries = malloc( sizeof( struct share_entry ) * 10 );
  struct share_entry entry = entries[0];
  
  return 0;
  
}

