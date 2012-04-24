#!/usr/bin/env python
#/*
#* WINHOMEFS This is a filesystem use for mouting
#* and sharing a windows home directory with a linux
#* instance without creating the big mess.
#* 
#* This file system is written by John-Charles D. Sokolow
#* Copyright (C) 2011-1012 John-Charles D. Sokolow <john.charles.sokolow@gmail.com>
#* 
#* Based heavily on the example fuse file system "fusexmp_fh.c"
#* Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
#* 
#* This program can be distributed under the terms of the GNU GPL.
#* See the file COPYING.
#* 
#*/

import subprocess, sys, os, shutil

def call( command ):
  
  c = subprocess.Popen( command.split(), stdout=subprocess.PIPE )
  c.wait()
  return c.stdout.read()

class GCC:
  
  args = None
  
  def __init__( self, initial_args ):
    
    self.args = initial_args
    
  def addPKG( self, package ):
    
    self.args.extend( package )
    
  def addFile( self, name ):
    
    self.args.append( name )
    
  def compile( self, out_name ):
    
    self.args.extend(["-o", out_name])
    print " ".join( self.args )
    gcc = subprocess.Popen( self.args )
    return gcc.wait() == 0
    
    
if __name__ == '__main__':
  
  cflags = call("pkg-config fuse --libs --cflags").split()
  
  print cflags
  
  gcc = GCC(["gcc","-g","-Wall","-pg"])
  gcc.addFile("argsparse.c")
  gcc.addFile("hidden.c")
  #gcc.addFile("fs.c")
  gcc.addFile("initialization.c")
  gcc.addFile("resolve.c")
  gcc.addFile("utilities.c")
  gcc.addFile("winhomefs0.4.c")
  gcc.addPKG(cflags)
  gcc.addFile("-lulockmgr")
  
  if gcc.compile("winhomefs") and 'install' in sys.argv:
    if os.getuid() == 0:
      shutil.copy("winhomefs", "/usr/local/bin/winhomefs")
      
      
      
    
