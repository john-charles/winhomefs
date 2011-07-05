#!/bin/sh
gcc `pkg-config fuse --cflags --libs` -g winhomefs.c -o winhomefs
