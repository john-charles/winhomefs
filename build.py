#!/usr/bin/env python

import subprocess

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
    gcc.wait()
    
if __name__ == '__main__':
  
  gcc = GCC(["gcc","-g", "-I./fs"])
  gcc.addPKG(call("pkg-config fuse --cflags --libs").split())
  gcc.addFile("argsparse.c")
  gcc.addFile("fs.c")
  gcc.addFile("initialization.c")
  gcc.addFile("resolve.c")
  gcc.addFile("utilities.c")
  gcc.addFile("winhomefs0.3.c")
  gcc.compile("winhomefs")