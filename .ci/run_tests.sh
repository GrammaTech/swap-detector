#!/bin/sh

set -e

mkdir build
cd build
cmake ../code/ -DLLVM_EXTERNAL_LIT=$(which lit) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH=/usr/lib/llvm-10/lib/cmake
cmake --build .
bin/TestSwappedArgsCpp
cmake --build . --target check-all
