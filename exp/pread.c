#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>


int main( void ){
  
  int input = open("/mnt/Personal/Users/john-charles/hello", O_RDWR );
  printf("after opening input is %i\n", input );
  printf("after opening errno is %i\n", errno );
  char * buff = (char*)malloc(4096);
  
  int read = pread( input, buff, 4096, 0 );
  printf("after reading read is  %i\n", read );
  printf("after reading errno is %i\n", errno );
  puts( buff );
  
  return 0;
  
}