#include "resolve.h"
#include "utilities.h"

#include <stdio.h>
#include <string.h>

extern char * root_path;
extern char * winversion;
extern char * winredirect;

extern redirect_t * my_documents;
extern redirect_t * my_music;
extern redirect_t * my_pictures;
extern redirect_t * my_videos;



char * resolve_vista( const char * path ){
  
  char * temp = 0;
  char * result = "";
  
  if( strncmp( path, "/.hidden", 8 ) == 0 ){
    
    result = cat( root_path, path, 0 );
    
  } else if( strncmp( path, "/.", 2 ) == 0 ){
    
    result = cat( winredirect, path, 0 );
    
  } else if( strncmp( path, my_videos->src, my_videos->len ) == 0 ){
    
    temp = cat( root_path, my_videos->dst, 0);
    result = cat( temp, substring( path, my_videos->len, strlen( path ) ), 1 );
    
  } else if( strncmp( path, my_music->src, my_music->len ) == 0 ){
    
    temp = cat( root_path, my_music->dst, 0 );
    result = cat( temp, substring( path, my_music->len, strlen( path ) ), 1 );
    
  } else if( strncmp( path, my_pictures->src, my_pictures->len ) == 0 ){
    
    temp = cat( root_path, my_pictures->dst, 0 );
    result = cat( temp, substring( path, my_pictures->len, strlen( path ) ), 1 );
    
  } else if( strncmp( path, my_documents->src, my_documents->len ) == 0 ){
    
    temp = cat( root_path, my_documents->dst, 0 );
    result = cat( temp, substring( path, my_documents->len, strlen( path ) ), 1 );
    
  } else {
    
    result = cat( root_path, path, 0 );
    
  }
  
  printf("resolve.c: resolve(%s) --> %s\n", path, result );
  
  return result;
  
}

char * resolve( const char * path ){
  
  if( strcmp( path, "/" ) == 0 ){
    
    return cat( root_path, "", 0 );
    
  }
  
  if( strcmp( winversion, "vista" ) == 0 ){
    
    return resolve_vista( path );
    
  } 
  
  if( strcmp( winversion, "win2k" ) == 0 ){
    
  }
  
  return 0;
  
}


// resolve_t * resolve( const char * path, int hidden ){
//   
//   puts("in resolve.");
//   
//   resolve_t * res = (resolve_t*)malloc( sizeof( resolve_t ) );
//   
//   puts("allocated resolve_t memory...");
//   
//   char * result = 0;
//   int size = 0;
//   
//   regmatch_t match[3];
//   
//   int failure = 1;
//   
//   memset( res, 0, sizeof( resolve_t ) );
//   
//   if( strcmp( path, "/" ) == 0 ){
//     
//     res->path = (char*)malloc( strlen( environment.real_root ) );    
//     
//     sprintf( res->path, "%s", environment.real_root );
//     
//     if( hidden ){
//       
//       res->hidden = root_hidden;
//       res->hidden_size = sizeof_root_hidden;
//       
//     }
//     
//     return res;
//     
//   }
//   
//   failure = regexec( &user_home_dir, path, 1, match, 0 );
//   
// //   if( !failure && hidden ){
// //       
// //       hidden = home_hidden;
// //       *hidden_size = sizeof_home_hidden;
// //   }
//   
//   memset( match, 0, sizeof( match ) );   
//   
//   failure = regexec( &user_dot_folder, path, 3, match, 0 );
//   
//   //puts("evaluated regular expression...");
//   
//   if( !failure ){
//     
//     //printf("Matched a regular expression...");puts("");
//     //printf("path is %s", path );puts("");
//     //printf("match[0].rm_eo = %i", match[0].rm_eo );puts("");
//     //printf("match[0].rm_so = %i", match[0].rm_so );puts("");
//     
//     size = match[1].rm_eo - match[1].rm_so;    
//     char * base = 0;
//     base = realloc( base, size );
//     
//     substring( base, path, match[1].rm_so, match[1].rm_eo );
//     
//     size = match[2].rm_eo - match[2].rm_so;
//     char * end = 0;
//     end = realloc( end, size );
//     
//     substring( end, path, match[2].rm_so, match[2].rm_eo );
//     
//     size = strlen( environment.real_root ) + strlen( base )
//      + strlen( environment.d_redirect )    + strlen( end ) + 3;
//     
//     result = realloc( result, size );
//     /* NOTE: base starts with a / so the second string does 
//      * not need to be followed by one */
//     sprintf( result, "%s%s/%s/%s", environment.real_root,
//              base, environment.d_redirect, end );
//     
//     free(base);base=0;
//     free(end);end=0;
//     
// //     if( hidden ){
// //       
// //       hidden = user_hidden;
// //       *hidden_size = sizeof_user_hidden;
// //     }
//     
//     
//     res->path = result;
//     
//     if( hidden ){
//       res->hidden = user_hidden;
//       res->hidden_size = sizeof_user_hidden;
//     }    
//     
//     return res;
//     
//   }
//   
// //   if( (int)hidden == 1 ){
// //     
// //     hidden = user_hidden;
// //     *hidden_size = sizeof_user_hidden;
// //   }
//   
//   size = strlen( environment.real_root ) + strlen( path );
//   result = realloc( result, size );
//   sprintf( result, "%s%s", environment.real_root, path );
//   
//   
//   res->path = result;
//   
//   if( hidden ){
//     res->hidden = user_hidden;
//     res->hidden_size = sizeof_user_hidden;
//   }    
//   
//   return res;
//   
// }