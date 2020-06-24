# Swap Detector

Module that checks for swapped arguments in function calls. For instance, the
library can be used to detect swaps in code like:
```c
/* Apparent swap of 'e' and 'n' based on parameter names. */
RSA_get0_key(rkey, &e, &n, NULL);
```

## Prerequisites
* CMake 3.10
* GCC 7 or Visual Studio 2017
* If enabling Clang plugin support: Clang 10 source

## Getting Started
### Setup for building Clang
* Check out Clang from git.
```bash
git clone https://github.com/llvm/llvm-project.git
git checkout llvmorg-10.0.0
```
* Build and install LLVM.
```bash
mkdir llvm-build; pushd llvm-build
cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_ASSERTIONS=1 -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_INSTALL_UTILS=1 -DLLVM_ENABLE_PROJECTS=clang -DLLVM_TOOL_CLANG_BUILD=1 -DCMAKE_INSTALL_PREFIX=$PWD/../llvm-install ../llvm-src
cmake --build . --target check-clang
cmake --build . --target install
popd
```
* [Install lit](https://pypi.org/project/lit/). Optional but required to be able to run tests.
```bash
pip install --user lit
export PATH=$PATH:$HOME/.local/bin
```
### Use cmake to build the plugin
```bash
mkdir build; cd build
cmake -G Ninja -DLLVM_EXTERNAL_LIT=$(which lit) -DSWAPPED_ARGS_BUILD_CLANG_PLUGIN=ON -DCMAKE_PREFIX_PATH=$PWD/../../llvm-install/lib/cmake ~/path/to/swap-detector
cmake --build . --target check-all
```

#### Notes

If you don't have Ninja installed, you can use `-G "Unix Makefiles"` to generate makefiles instead and build using `make -j`.

There is a linker warning about use of `tmpnam`. This API is only used by the testing infrastructure to generate a temporary statistics database, and is not used as part of the swapped argument checker API.

#### Example

```bash
../../llvm-install/bin/scan-build -load-plugin lib/SwapDetectorPlugin.so -enable-checker gt.SwapDetector -analyzer-config gt.SwapDetector:ModelPath=sample.db clang++ ~/dummy.cpp
```
The root directory of the repository has a sample database, named `sample.db`,
which can be used to explore the behavior of the library. This database is not
complete (it only covers ten functions), but does contain statistically useful
information about the functions it covers.

### Configuration Options
Option | Description
------ | -----------
`SWAPPED_ARGS_BUILD_CLANG_PLUGIN` | Enables building the Clang plugin. Default: ON
`SWAPPED_ARGS_BUILD_TESTS` | Enables building tests. Default: ON
`SWAPPED_ARGS_BUILD_PYTHON` | Enables building the Python extension. Default: Off
`SWAPPED_ARGS_INSTALL_PYTHON` | Enables installing the Python extension if it's been built. Default: Off

### Automatic Downloads
As part of the CMake configuration, the latest master branch of [googletest](https://github.com/google/googletest) is downloaded and built if testing
functionality is enabled.

### Testing
To run the C++ unit tests, ensure that `SWAPPED_ARGS_BUILD_TESTS` is not
disabled when configuring the cmake project. The `TestSwappedArgsCpp` executable
will be generated on successful build and can be run to perform unit testing.

To run the Clang plugin tests, you can execute ``cmake --build . --target check-all`` from the CMake build directory.

#### Acknowledgements
This material is based on research sponsored by the Department of Homeland
Security (DHS) Office of Procurement Operations, S&T acquisition Division via
contract number 70RSAT19C00000056. The views and conclusions contained herein
are those of the authors and should not be interpreted as necessarily
representing the official policies or endorsements, either expressed or
implied, of the Department of Homeland Security.

