#!/bin/bash

rm -f CMakeCache.txt
export CFLAGS="-DPAWN_CELL_SIZE=32"
cmake ../../source/compiler/
make
