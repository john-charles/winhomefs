#include <fuse_opt.h>
#include <string.h>
#include <stdlib.h>

/** options for fuse_opt.h */
struct options {
   char* foo;
   char* bar;
   int baz;
   double quux;
}options;

/** macro to define options */
#define HELLOFS_OPT_KEY(t, p, v) { t, offsetof(struct options, p), v }

/** keys for FUSE_OPT_ options */
enum
{
   KEY_VERSION,
   KEY_HELP,
};

static struct fuse_opt hello_opts[] =
{      
  FUSE_OPT_KEY("-v",             KEY_VERSION),
  FUSE_OPT_KEY("--version",      KEY_VERSION),
  FUSE_OPT_KEY("-h",             KEY_HELP),
  FUSE_OPT_KEY("--help",         KEY_HELP),
  FUSE_OPT_END
};

char * preparse_opts( int * argc, char * argv[] ){
  
  char * root = 0;
  
  int i;
  
  for( i = 1; i < *argc; i++ ){
    
    if( strcmp( argv[i], "-o" ) == 0 ){
      /* We can skip the next command */
      i++;
    
      
    } else if( argv[i][0] == '-' ){
      continue;
      
    } else if( root == 0 ){
      
      root = (char*)malloc( strlen( argv[i] ) );
      strcpy( root, argv[i] );
      i--;
      
    } else {
      
      if( ( i + 1 ) < *argc ){
        
        argv[i] = argv[i+1];
        
      }
    }
  }
  
  *argc = *argc - 1;
  return root;
  
}
      
      

//   char * map_dir = 0;
//   int i;
//   for (i = 1; (i < argc) && (argv[i][0] == '-'); i++){
//     if (argv[i][1] == 'o'){
//       i++;
//       // -o takes a parameter; need to
//       // skip it too.  This doesn't
//       // handle "squashed" parameters
//     } else if ((argc - i) != 2){
//       puts("Error parsing command arguments...");
//       return 0;
//     } else {
//       
//       
//     
//     map_dir = argv[i];
//     argv[i] = argv[i + 1];
//     argc--;
//     
//   }
//   
//   for(i = 0; i < argc; i++ ){
//     puts( argv[i] );
//   }


int parse( struct fuse_args * args, int argc, char * argv[] ){
  
  
  /* clear structure that holds our options */
  memset(&options, 0, sizeof(struct options));
  
  if (fuse_opt_parse(args, &options, hello_opts, NULL) == -1)
    /** error parsing options */
    return 0;
  
  return 1;
  
}