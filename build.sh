#!/bin/sh
g++ -std=c++0x  `pkg-config fuse --cflags --libs` winhomefs.cpp -o winhomefs
