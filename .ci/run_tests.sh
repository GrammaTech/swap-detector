#!/bin/sh

set -e

mkdir build
cd build
cmake ../code/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH=/usr/lib/llvm-10/lib/cmake
cmake --build .
bin/TestSwappedArgsCpp
