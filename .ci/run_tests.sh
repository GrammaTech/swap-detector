#!/bin/bash

set -e

mkdir build
pushd build
cmake ../code/ -DLLVM_EXTERNAL_LIT=$(which lit) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH=/usr/lib/llvm-10/lib/cmake -DSWAPPED_ARGS_BUILD_PYTHON=ON
cmake --build .
cmake --install .
bin/TestSwappedArgsCpp
cmake --build . --target check-all
popd

pytest code/python
