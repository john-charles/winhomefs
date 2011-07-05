#!/bin/sh
gcc `pkg-config fuse --cflags --libs` -g utilities.c initialization.c winhomefs0.3.c -o winhomefs
