#!/bin/sh
gcc `pkg-config fuse --cflags --libs` winhomefs.c -o winhomefs
