/*
 * WINHOMEFS This is a filesystem use for mouting
 * and sharing a windows home directory with a linux
 * instance without creating the big mess.
 * 
 * This file system is written by John-Charles D. Sokolow
 * Copyright (C) 2011-1012 John-Charles D. Sokolow <john.charles.sokolow@gmail.com>
 * 
 * Based heavily on the example fuse file system "fusexmp_fh.c"
 * Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
 * 
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 * 
 */
#ifndef INITIALIZATION_H
#define INITIALIZATION_H

extern char * root_path;
extern char * winversion;

int initialize_environment( char * profile_directory );
int initialize_redirect( );
int initialize_regex( );
int initialize_default_hidden_lists( );

#endif