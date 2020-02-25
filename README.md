# swapped-arg-checker

Module that checks for swapped arguments in function calls.

## Prerequisites
* CMake 3.14
* GCC 7 or Visual Studio 2017

## Getting Started
To get started, run cmake to generate the appropriate files for your target
build system. Currently, only out-of-tree builds are supported. For instance,
assuming that the repository is cloned to a directory named
`swapped-arg-checker`:
```
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 -Thost=x64 ..\swappged-arg-checker
```
-or-
```
cmake -G "Unix Makefiles" ../swapped-arg-checker
```

### Configuration Options
Option | Description
------ | -----------
`SWAPPED_ARGS_BUILD_TESTS` | Enables building tests. Default: ON
`SWAPPED_ARGS_BUILD_SHARED_LIBS` | Enables building a shared library instead of a static library. Default: ON
`SWAPPED_ARGS_BUILD_PYTHON` | Enables building Python SWIG bindings. Default: OFF

### Automatic Downloads
As part of the CMake configuration, the latest master branch of [googletest]
(https://github.com/google/googletest) is downloaded and built if testing
functionality is enabled. Additionally, [SWIG 4.0.1](http://www.swig.org/) is
downloaded and built if other language support is enabled.

### Testing
To run the C++ unit tests, ensure that `SWAPPED_ARGS_BUILD_TESTS` is not
disabled when configuring the cmake project. The `TestSwappedArgsCpp` executable
will be generated on successful build and can be run to perform unit testing.
