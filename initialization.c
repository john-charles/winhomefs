#include "utilities.h"
#include "initialization.h"

#include <stdio.h>
#include <dirent.h>
#include <regex.h>

extern char * root_path;
extern char * winversion;
extern char * winredirect;

int initialize_environment( char * profile_directory ){
  
  
  char * temp = 0;
  list_t * contents = list_t_new_listdir( profile_directory );
  int success = 0;
  
  if( contents->length ){
    
    if( list_t_contains( contents, "Users" ) ){
     
      puts("Found users dir, assuming windows vista and above!");
      temp = add( profile_directory, "/" );
      root_path = add( temp, "Users" );
      free( temp );
      winversion = "vista";
      winredirect = "AppData/Linux";
      success = 1;
          
    } else if( list_t_contains( contents, "Documents and Settings" ) ){
      
      puts("Found Documents and Settings dir, assuming windows 2000 and above!");
      temp = add( profile_directory, "/" );
      root_path = add( temp, "Documents and Settings" );
      free( temp );
      winversion = "win2k";
      winredirect = "Linux Data";
      success = 1;
      
    }     
  } 
  
  list_t_free( contents );
  
  if( !success ){
    
    puts("Could not profile directory, sorry!");
    
  }
  
  return success;   
   
}

extern regex_t regex_user_home_dir;
extern regex_t regex_user_dot_folder;
extern regex_t regex_user_doc_subdir;
extern regex_t regex_user_mdocs_dir;

int initialize_regex( ){
  
  int failure;
  
  failure = regcomp( &regex_user_home_dir, "^/[^/]+$", REG_EXTENDED );
  
  if( failure ){
    
    puts("Failed to compile homedir expression!");
    return 0;
    
  }
  
  failure = regcomp( &regex_user_dot_folder, "(^/[^/]+)/(\\..*$)", REG_EXTENDED );
  
  if( failure ){
    
    puts("Failed to compile user_dot_folder regex...");
    return 0;
    
  }
    
  failure = regcomp( &regex_user_doc_subdir, "(^/.*)/(My Documents/My [Music|Pictures|Videos]+)/(.*$)", REG_EXTENDED );
  
  if( failure ){
    
    puts("Failed to compile user_documentsf reges...");
    return 0;
    
  }
  
  failure = regcomp( &regex_user_mdocs_dir, "(^/.*)/(My Documents)/(.*$)", REG_EXTENDED );
  
  if( failure ) {
    
    puts("Failed to compile user_docs_dir regex...");
    return 0;
    
  }
  
  return 1;
  
};

extern list_t * hidden_list_root;
extern list_t * hidden_list_home;
extern list_t * hidden_list_user;

int initialize_default_hidden_lists( ){
  
  hidden_list_root = list_t_new();
  hidden_list_home = list_t_new();
  hidden_list_user = list_t_new();
  
  list_t_append( hidden_list_root, "All Users" );
  list_t_append( hidden_list_root, "Default User" );
  list_t_append( hidden_list_root, "Default" );
  list_t_append( hidden_list_root, "Public" );
  
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
  
  
  
  