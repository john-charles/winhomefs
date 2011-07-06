#include "utilities.h"

#ifndef RESOLVE_H
#define RESOLVE_H

typedef struct {
  
  char * src;
  char * dst;
  int    len;
  
} redirect_t;


char * resolve( const char * path );

#endif