#include "utilities.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stddef.h>

/** Private functions **/

int compare_strings( const void * a, const void * b ){
  
  const char ** sa = ( const char ** ) a;
  const char ** sb = ( const char ** ) b;
  
  return strcmp( *sb, *sa );
   
}

void qsort_strings( char * list[], int count ){
  
  qsort( list, count, sizeof( char * ), compare_strings );
  
}

char * bsearch_strings( char * term, char * list[], int count ){
  
  char * term_array[] = { term };
  char ** result;
  
  result = bsearch( term_array, list, count, sizeof( char * ), compare_strings );
  
  if( result ){
    
    return * result;
    
  }
  
  return 0;
  
}

/** Public functions, by this I mean defined in the header file utilities.h **/

char * substring( const char * string, int start, int end ){
  
  int i, j, new_size = end - start + 1;
  
  char * result = (char*)malloc(new_size);
  
  for( i = start, j = 0; i < end; i++, j++ ){
    
    result[j] = string[i];
    
  }
  
  result[j] = '\x00';
  
  return result;
  
}

int startswith( const char * string, const char * term ){
  
  int t_length = strlen( term );
  
  if( strlen( string ) >= t_length ){
    
    char * temp = substring( string, 0, t_length );
    
    int eq = strcmp( term, temp );
    
    free( temp );
    
    return eq == 0;
    
  }
  
  return 0;
  
}
  
  

char * cat( char * str1, const char * str2, int do_free ){
  
  char * new = 0;
  
  if( str1 != 0 ){
    
    new = (char*)malloc( strlen(str1) + strlen(str2) + 1 );
  
    strcpy( new, str1 );
    strcat( new, str2 );
    
    if( do_free ){
      
      free( str1 );
    }
  
    
  } else {
    
    new = (char*)malloc( strlen( str2 ) );
    
    strcpy( new, str2 );
    
  }
    
  return new;
  
}
  
  

void list_t_sort( list_t * list ){
  
  if( list->sorted == 0 ){
    /* NOTE: If this starts segfaulting I remember reading
     * somewhre that qsort might re-arrange the size or something...
     */
    
    qsort_strings( list->data, list->length );
    list->sorted = 1;
    
  }
  
}

list_t * list_t_new( ){
  
  list_t * new_list = (list_t*)malloc( sizeof( list_t ) );
  memset( new_list, 0, sizeof( new_list ) );
  new_list->length = 0;
  new_list->data = 0;
  new_list->sorted = 0;
  
  return new_list;
  
}

list_t * list_t_new_from_file( char * path ){
  
  list_t * new_list = list_t_new();
  FILE   * file = fopen( path, "rb" );
  
  int    read = 0;
  int    leng = 0;
  size_t size = 20;
  char * buff = (char*)malloc( size );
  char * temp = 0;
  
  if( file ){
    
    read = getline( &buff, &size, file );
    
    while( read != -1 ){
      
      leng = strlen( buff );
      
      if( buff[ leng -1 ] == '\n' ){
        temp = substring( buff, 0, leng - 1 );
      } else {
        temp = substring( buff, 0, leng );
      }
      
      list_t_append( new_list, temp );
      
      free( temp ); temp = 0;
      
      read = getline( &buff, &size, file );
      
    }
    
    fclose( file );
  }
  free( buff );
  return new_list;
  
}

list_t * list_t_new_listdir( const char * path ){

  DIR           * directory = opendir( path );
  struct dirent * entry;
  list_t        * contents = list_t_new();
  
  if( directory ){
    
    while(( entry = readdir( directory ) )){
      
      list_t_append( contents, entry->d_name );
            
    }
  }
  
  closedir( directory );
  list_t_sort( contents );
  return contents;
  
}
    

void list_t_append( list_t * list, char * string ){
  
  if( list->length == 0 ){
    
    list->data = (char**)malloc( sizeof( char**) );
    list->data[0] = substring( string, 0, strlen( string ) );
    list->length = 1;
    list->sorted = 0;
    
  } else {
    
    // Because realloc gives me all sorts of trouble...
    int new_size = sizeof(char**) * (list->length + 1);
    char ** temp = (char**)malloc( new_size );
    
    memcpy( temp, list->data, sizeof(char**) * list->length );
    
    free( list->data );
    
    list->data = temp;    
    list->data[ list->length++ ] = substring( string, 0, strlen( string ) );
    list->sorted = 0;
  }
  
}

int list_t_contains( list_t * list, char * string ){
  
  if( list->sorted == 0 ){
    
    int i;
    
    for( i = 0; i < list->length; i++ ){
      
      if( strcmp( list->data[i], string ) == 0 ){
        return 1;
      }
    }    
  } else {
    
    return bsearch_strings( string, list->data, list->length ) != 0;
    
  }
  
  return 0;
  
}

void list_t_append_unique( list_t * list, char * string ){
  
  if( !list->sorted ){
    list_t_sort( list );
  }
  if( !list_t_contains( list, string ) ){
    
    list_t_append( list, string );
    list_t_sort( list );
    
  }
  
}





void list_t_extend( list_t * list, list_t * second ){
  
  int i;
  
  for( i = 0; i < second->length; i++ ){
    
    list_t_append( list, second->data[i] );
    
  }
}

void list_t_extend_unique( list_t * list, list_t * second ){
  
  int i;
  
  for( i = 0; i < second->length; i++ ){
    
    list_t_append_unique( list, second->data[i] );
    
  }
}

void list_t_print( list_t * list ){
  
  int i;
  printf("[ ");
  for( i = 0; i < list->length; i++ ){
    printf("\"%s\"",list->data[i]);
    if( i+1 < list->length ){
      printf(", ");
    }
  }
  printf(" ]");
  
}

void list_t_printf( FILE * file, list_t * list ){
  
  int i;
  fprintf(file,"[ ");
  for( i = 0; i < list->length; i++ ){
    fprintf(file,"\"%s\"",list->data[i]);
    if( i+1 < list->length ){
      fprintf(file,", ");
    }
  }
  fprintf(file," ]");
  
}

void list_t_free( list_t * list ){
  
  int i;
  
  for( i = 0; i < list->length; i++ ){
    
    free( list->data[i] );
    
  }
  
  free( list->data );
  free( list );
  
}

char * add( const char * str1, const char * str2 ){
  
  char * new = (char*)malloc( strlen(str1) + strlen(str2) + 1 );
  
  strcpy( new, str1 );
  strcat( new, str2 );
  
  return new;
  
}
    
    
    
    
char * join( list_t * path, char * delim ){
  
  char * result = add("","");
  char * temp = 0;
  int i;
  
  for( i = 0; i < path->length; i++ ){
    temp = result;
    result = add( result, delim );
    free(temp);
    
    temp = result;
    result = add( result, path->data[i] );
    free(temp );
    
  }
  
  return result;
  
}
    
    
  
  
    
FILE * log_f( const char * title, const char * message ){
  char * base = "/tmp/";
  char * path = (char*)malloc( strlen( base ) + strlen( title ) );
  
  sprintf( path, "%s%s", base, title );
  
  FILE * f = fopen(path,"ab");
  
  
  return f;
   
}
  
  
  

  
  
  
  

  

    
    