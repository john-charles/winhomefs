#include "utilities.h"
#include "resolve.h"
#include "initialization.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <regex.h>

extern char * root_path;
extern char * winredirect;
extern char * winversion;

int validate_dir( char * path ){
  
  int success = mkdir( path, 0700 );
  
  if( success == -1 ){
    
    if( errno == EEXIST ){
      
      return 1;
      
    } else {
      
      return 0;
      
    }
  }
  
  return 1;
  
}

int initialize_environment( char * profile_directory ){
  
  int success = 1;
  
  char * redirect = 0;
  
  list_t * contents = list_t_new_listdir( profile_directory );
    
  if( list_t_contains( contents, "AppData" ) ){
    
    redirect = cat( profile_directory, "/AppData/Linux", 0);
    
    if( !validate_dir( redirect ) ){
      
      printf("Error could not create directory %s\n", redirect );
      success = 0;
    
      
    } else {
      
      winredirect = redirect;
      winversion = "vista";
      
    }
    
  } else if( list_t_contains( contents, "Application Data" ) ){
    
    redirect = cat( profile_directory, "/Linux Data", 0 );
    
    if( !validate_dir( redirect ) ){
      
      printf("Error could not create directory %s\n", redirect );
      success = 0;
      
    } else {
      
      winredirect = redirect;
      winversion = "win2k";
      
    }
  
  } else {
    
    success = 0;
    
  }
  
  if( success ){
    
    printf("validated directory %s\n", profile_directory );
    printf("validated directory %s\n", winredirect );
    printf("validated winversion %s or above!\n", winversion );
    root_path = profile_directory;
    
  } else {
    
    printf("failed! %s does not appear to be a valid windows profile!\n", profile_directory );
    
  }
  
  return success;
   
}

extern redirect_t * my_documents;
extern redirect_t * my_music;
extern redirect_t * my_pictures;
extern redirect_t * my_videos;

redirect_t * populate( const char * src, const char * dst ){
  
  redirect_t * new = (redirect_t*)malloc( sizeof( redirect_t ) );
  
  new->src = cat((char*)src,"",0);
  new->dst = cat((char*)dst,"",0);
  new->len = strlen( src );
  
  return new;
  
}

int initialize_redirect( ){
  
  if( strcmp( winversion, "vista" ) == 0 ){
    
    my_documents = populate("/My Documents","/Documents");
    my_music     = populate("/My Documents/My Music","/Music");
    my_pictures  = populate("/My Documents/My Pictures","/Pictures");
    my_videos    = populate("/My Documents/My Videos","/Videos");
    
  } else {
    
    my_documents = populate("/Documents","/My Documents");
    my_music     = populate("/Music","/My Documents/My Music");
    my_pictures  = populate("/Pictures","/My Documents/My Pictures");
    my_videos    = populate("/Videos","/My Documents/My Videos");
    
  }
  
  
  
    
    
    
  
  
  return 1;
  
}

int initialize_regex( ){
//   
//   int failure;
//   
//   failure = regcomp( &regex_user_home_dir, "^/[^/]+$", REG_EXTENDED );
//   
//   if( failure ){
//     
//     puts("Failed to compile homedir expression!");
//     return 0;
//     
//   }
//   
//   failure = regcomp( &regex_user_dot_folder, "(^/[^/]+)/(\\..*$)", REG_EXTENDED );
//   
//   if( failure ){
//     
//     puts("Failed to compile user_dot_folder regex...");
//     return 0;
//     
//   }
//     
//   failure = regcomp( &regex_user_doc_subdir, "(^/.*)/(My Documents/My [Music|Pictures|Videos]+)/(.*$)", REG_EXTENDED );
//   
//   if( failure ){
//     
//     puts("Failed to compile user_documentsf reges...");
//     return 0;
//     
//   }
//   
//   failure = regcomp( &regex_user_mdocs_dir, "(^/.*)/(My Documents)/(.*$)", REG_EXTENDED );
//   
//   if( failure ) {
//     
//     puts("Failed to compile user_docs_dir regex...");
//     return 0;
//     
//   }
//   
  return 1;
   
};

extern list_t * hidden_list_root;
extern list_t * hidden_list_home;
extern list_t * hidden_list_user;

int initialize_default_hidden_lists( ){
  
  
  hidden_list_home = list_t_new();
  hidden_list_user = list_t_new();
    
  list_t_append( hidden_list_home, "AppData" );
  list_t_append( hidden_list_home, "Application Data" );
  list_t_append( hidden_list_home, "Cookies" );
  list_t_append( hidden_list_home, "Local Settings" );
  list_t_append( hidden_list_home, "NetHood");
  list_t_append( hidden_list_home, "ntuser.*");
  list_t_append( hidden_list_home, "NTUSER.*");
  list_t_append( hidden_list_home, "PrintHood" );
  list_t_append( hidden_list_home, "Recent" );
  list_t_append( hidden_list_home, "SendTo" );
  list_t_append( hidden_list_home, "Start Menu" );
  list_t_append( hidden_list_home, "Templets" );
  
  list_t_append( hidden_list_user, "desktop.ini" );
  list_t_append( hidden_list_user, "Desktop.ini" );
  list_t_append( hidden_list_user, "Thumbs.db"   );
  
  return 1;
  
}
  
  
  
  