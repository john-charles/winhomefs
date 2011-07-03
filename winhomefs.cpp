/* STD Headers... */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

/* C++ headers.... I broke down, c++ is easier for some things... */
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

/* Fuse specific headers... */
#include <fuse.h>
#include <fuse_opt.h>

/* Import unix file managment headers */
#include <sys/errno.h>
#include <sys/stat.h>
#include <dirent.h>

/* Fuse option_handling....*/

/* Configuration opject for the entire fs.... */
static struct winhomefs_config {
  char * windows_home_dir;
} winhomefs_config;

enum {
     KEY_HELP,
     KEY_VERSION,
};

#define WINHOMEFS_OPT(t, p, v) { t, offsetof(struct winhomefs_config, p), v }

/* Let's define our fuse options... */
static struct fuse_opt fs_options[] = {
  
  WINHOMEFS_OPT("root=%s", windows_home_dir, 0 ),
  
};

int parse_fuse_args( int argc, char ** argv ){
  
  // Clear the structure...
  memset( &winhomefs_config, 0, sizeof( winhomefs_config ) );
  struct fuse_args args = FUSE_ARGS_INIT( argc, argv );
  int success = fuse_opt_parse(&args, &winhomefs_config, fs_options, NULL );
  if( !winhomefs_config.windows_home_dir ){
    success = -1;
    printf("Please specifiy the path to your windows user profile\n");
    printf("Using the -o root=/some/path option!\n");
  }
  
  return success;
  
}

/** Begin utilities for the actual file system **/

int str_endswith( std::string fullString, std::string ending ){

  
  if (fullString.length() > ending.length()) {
    
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    
  } else {
    
    return 0;
    
  }
  
}

using namespace std;

bool exists( string path ){
  struct stat * buf = ( struct stat * ) malloc( sizeof( struct stat ) );
  int result = stat( path.c_str(), buf );
  delete buf;
  if ( result == 0 ){
    return 1;
  }
  
  return 0;
  
}

class WinHomeFS {
  
  string root;
  string winversion;
  string dirprefix;
  
  map<string,string> vista_symlinks;
  map<string,string> win2k_symlinks;
  
  map<string,vector<string>*> hidden;
  
  map<string,string> resolution_cache;
  
  
  vector<string> splitpath( char * path ){
     
     vector<string> result;
     string temp;
     int len = strlen( path );
     char this_char = ' ';
     
     for( int i = 1; i < len; i++ ){
       
       this_char = path[i];
       
       if( this_char == '/' && temp.size() > 0 ){
         
         result.push_back( temp );         
         temp = "";
         
       } else {
         
         temp = temp + this_char;
         
       }
                
     }
     
     result.push_back( temp );
     
     
     return result;
     
   }
  
  string win2k_resolve( char * c_path ){
    
    return 0;
    
  }
  
  string vista_resolve( char * c_path, int * is_home, int * is_root ){
    
    string path = this->root + this->dirprefix;
    vector<string> parts = this->splitpath( c_path );
//     cout << "parts[0] == \"" << parts[0] << "\"" << endl;
    if( parts.size() == 1 && ( parts[0] == "" || parts[0] == ".") ){
      *is_root = 1;
    }
    if( parts.size() == 1 && ( parts[0] != "" && parts[0] != "." ) ){
      
      *is_home = 1;
      
    }
        
    if( parts.size() >= 2 ){
      
      if( parts[1][0] == '.' ){
        
        parts[0] = parts[0] + "/AppData/Linux";
        
      }
    }
    
    for( vector<string>::iterator it = parts.begin(); it != parts.end(); it++ ){
      
      path = path + "/" + *it;
      
    }   
    
    string end = path.substr( path.length() -2 );
        
    if( end == "/." ){
      return path.substr( 0, path.length() -2 );
    } if( end[1] == '/' ){
      return path.substr(0, path.length() -1 );
    }
    
    return path;
        
  }
  
  void prep_dir( void ){
    
    string path = this->resolve((char*)"/");
    DIR * dp = opendir( path.c_str() );
    
    struct dirent * entry = readdir( dp );
    
    string entry_name;
    string linux_dir = "";
    
    
    while( entry != NULL ){
      
      entry_name = string( entry->d_name );
      
      if( entry_name != "." && entry_name != ".." ){
        
        entry_name = "/" + entry_name + "/.";
        linux_dir = resolve((char*)entry_name.c_str() );
        
        if( !exists( linux_dir ) ){
          
          mkdir( linux_dir.c_str(), 0750 );
          
        }
        
        
      }
      
      entry = readdir(dp);
      
    }
    
    closedir(dp);
    
          
  }
  
  vector<string> home_hidden;
  vector<string> root_hidden;
  vector<string> user_hidden;
  
public:
  
  WinHomeFS( char * root_dir ){
    
    this->root = string( root_dir );
    if( exists( this->root + "/Users" ) ){
      
      this->winversion = string("vista");
      this->dirprefix  = string("/Users");   
      
      home_hidden.push_back("AppData");
      home_hidden.push_back("Application Data");
      home_hidden.push_back("Cookies");
      home_hidden.push_back("Local Settings");
      home_hidden.push_back("My Documents");
      home_hidden.push_back("My Music");
      home_hidden.push_back("My Videos");
      home_hidden.push_back("My Pictures");
      home_hidden.push_back("NetHood");
      home_hidden.push_back("NTUSER.DAT");
      home_hidden.push_back("ntuser.dat.*");
      home_hidden.push_back("NTUSER.DAT*");
      home_hidden.push_back("ntuser.ini");
      home_hidden.push_back("PrintHood");
      home_hidden.push_back("Recent");
      home_hidden.push_back("SendTo");
      home_hidden.push_back("Start Menu");
      
      root_hidden.push_back("All Users");
      root_hidden.push_back("Default");
      root_hidden.push_back("Default User");
      root_hidden.push_back("desktop.ini");
      root_hidden.push_back("Desktop.ini");
      root_hidden.push_back("Public");
      
      user_hidden.push_back("Desktop.ini");
      user_hidden.push_back("desktop.ini");
      user_hidden.push_back("Thumbs.db");
      
      
    } else {
      
      this->winversion = string("2000");
      this->dirprefix  = string("/Documents and Settings");
      
    }
    
    prep_dir(); 
    
  }
  
  
  const char * resolve( const char * c_path ){
    
    string path = string( c_path );
    string result;
    map<string,string>::iterator it = this->resolution_cache.find( path );
    
    if( it != this->resolution_cache.end() ){
      
      result = it->second;
      
    } else {
      
      int is_home = 0, is_root=0;
      
      if( this->winversion == "2000" ){
        
        result = this->win2k_resolve( (char*)path.c_str() );
        
        
      } else if( this->winversion == "vista" ){
        
        result =  this->vista_resolve((char*) path.c_str(), &is_home, &is_root );
        
      }
      
      if( is_home ){
        
        cout << result << " is a home dir..." << endl;        
        this->hidden[ result ] = &home_hidden;
        
      } else if( is_root ){
        
        cout << result << " is the root..." << endl;
        
        this->hidden[ result ] = &root_hidden;
        
      }
      
      this->resolution_cache[ path ] = result;
      
    }
    
    return result.c_str();
    
  }
  
  int fuse_getattr( const char * path, struct stat * stbuf ){
    
    path = this->resolve( path );
    
    memset(stbuf, 0, sizeof(struct stat));
    
    int code = stat( path, stbuf );
    
    if( code == 0 ){
      
      return code;
      
    }
    
    return -ENOENT;
          
  }
  
  vector<string> gethidden( const char * path ){
    
    const char * hidden_file = string( string( path ) + "/.hidden" ).c_str();
    
    vector<string> hidden_list;
    
    if( this->hidden.count( string( path ) ) ){
      
      vector<string> * default_hidden = this->hidden[ string( path ) ];
      vector<string>::iterator it = default_hidden->begin();
      for(; it != default_hidden->end(); it++ ){
        
        hidden_list.push_back( *it );
        
      }
      
    }
      
    ifstream in( hidden_file );
    string temp;
    
    if( in.is_open() ){
      
      while( !in.eof() ){
        
        in >> temp;
        hidden_list.push_back( temp );
        
      }
    }
    
    in.close();    
     
    return hidden_list;
  
    
  }
    
  
   int fuse_readdir( const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
     
     //NOTE: I don't know what nay other options are for, other than the fact that they are here...
     path = this->resolve( path );
     DIR * dp = opendir( path );
     if( dp == 0 ) return -EPERM;
     struct dirent * entry;
     vector<string> h = gethidden( path );
     vector<string>::iterator it;
     string d_name;
     bool found=false;
     
     struct stat * stbuf = (struct stat *)malloc( sizeof(struct stat) );
     
     
     while(( entry = readdir(dp))){
       found = false;
       d_name = string( entry->d_name );
       for( it = h.begin(); it != h.end(); it++){
         if( d_name == *it ) found = true;
       }
       if( !found ){
         memset(stbuf, 0, sizeof(struct stat));
         stat( string( string( path ) + "/" + d_name ).c_str(), stbuf );
         filler( buf, entry->d_name, stbuf, 0  );
       
         
      }
     }
     
     closedir( dp );     
  }
  
  void print( void ){
    
    cout << "WinHomeFS V0.0" << endl;
    cout << "resolution cache size: " << this->resolution_cache.size() << endl;
    
  }
  
};
  


int main_fs( int argc, char ** argv ){
  
  
  
  int code = parse_fuse_args(argc, argv);
  if( code != 0 ) return code;
  
     
    
  WinHomeFS fs = WinHomeFS( winhomefs_config.windows_home_dir );
  
//   cout << fs.resolve((char*)"/") << endl;
   cout << fs.resolve((char*)"/.") << endl;
   cout << fs.resolve((char*)"/john-charles") << endl;
//   cout << fs.resolve((char*)"/john-charles/.") << endl;
   cout << fs.resolve((char*)"/john-charles/.config") << endl;
//   cout << fs.resolve((char*)"/john-charles/Projects") << endl;
//   
  
  /** Testing getattr... **/
  
  struct stat * stbuf = (struct stat *) malloc( sizeof( struct stat ) );
  
  int success = fs.fuse_getattr((const char*)"/", stbuf );
  
  cout << "getattr return value: " << success << endl;
  cout << "getattr stbuf->st_mode:  " << stbuf->st_mode << endl;
  cout << "getattr stbuf->st_size:  " << stbuf->st_size << endl;
  cout << "getattr stbuf->st_mtime: " << stbuf->st_mtime << endl;
  
  fs.print();
  
  struct fuse_operations fsOpts;
  
  fsOpts.getattr = fs.fuse_getattr;
  fsOpts.readdir = fs.fuse_readdir;
  
  return fuse_main( argc, argv, &fsOpts  );
  
}

int main( int argc, char ** argv ){
  
    
  main_fs( argc, argv );
  
}