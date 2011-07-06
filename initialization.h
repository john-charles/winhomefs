

#ifndef INITIALIZATION_H
#define INITIALIZATION_H

extern char * root_path;
extern char * winversion;

int initialize_environment( char * profile_directory );
int initialize_redirect( );
int initialize_regex( );
int initialize_default_hidden_lists( );

#endif