#!/bin/bash

set -e

mkdir build
pushd build
cmake ../code/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DSWAPPED_ARGS_BUILD_PYTHON=ON -DSWAPPED_ARGS_BUILD_SHARED_LIBS=OFF
cmake --build .
cmake --install .
bin/TestSwappedArgsCpp
popd

pytest code/python
