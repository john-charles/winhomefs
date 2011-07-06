#!/bin/sh
gcc `pkg-config fuse --cflags --libs` -g \
  fs/readdir.c \
  argsparse.c \
  fs.c \
  utilities.c \
  initialization.c \
  resolve.c \
  winhomefs0.3.c \
  -o winhomefs
