#!/bin/sh

set -e

mkdir build
cd build
cmake ../code/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DSWAPPED_ARGS_BUILD_PYTHON=ON
cmake --build .
bin/TestSwappedArgsCpp
