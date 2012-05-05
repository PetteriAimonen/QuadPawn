#!/bin/bash

# This script cross-compiles the pawn compiler using mingw.
# Install the ubuntu/debian package 'mingw32' to use it.

rm -f CMakeCache.txt
cmake -DCMAKE_TOOLCHAIN_FILE=mingw.cmake ../source/compiler/
make
