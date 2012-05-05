#!/bin/bash

rm -f CMakeCache.txt
export CFLAGS="-DHAVE_I64"
cmake ../source/compiler/
make
