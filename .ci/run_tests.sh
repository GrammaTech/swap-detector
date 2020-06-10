#!/bin/sh

set -e

mkdir build
cd build
cmake ../code/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH=../../llvm-install/lib/cmake
cmake --build .
bin/TestSwappedArgsCpp
