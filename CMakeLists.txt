# Need 3.14 to support UseSWIG
cmake_minimum_required(VERSION 3.14.0)

# Enable project versions.
cmake_policy(SET CMP0048 NEW)

# Use the SWIG_MODULE_NAME property.
cmake_policy(SET CMP0086 NEW)

project(
  SwappedArgChecker
  VERSION "0.0.1"
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
# Error if it's not available
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifically check for gcc-7 or later. gcc-5 is installed on many systems
# and will accept -std=c++17, but does not fully support the standard.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0.0")
    message(FATAL_ERROR "gcc 7 or later is required")
  endif()
endif()

# This just sets the builtin BUILD_SHARED_LIBS, but if defaults to ON instead of
# OFF.
option(SWAPPED_ARGS_BUILD_SHARED_LIBS "Build shared libraries." ON)
if(SWAPPED_ARGS_BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
else()
  set(BUILD_SHARED_LIBS OFF)
endif()
if(UNIX AND NOT BUILD_SHARED_LIBS)
  # Find only static libraries
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  add_compile_options(-static)
endif()

add_subdirectory(src)

option(SWAPPED_ARGS_BUILD_PYTHON "Build python SWIG bindings." OFF)

if(SWAPPED_ARGS_BUILD_PYTHON)
  message(STATUS "Getting SWIG: ...")
  # Download and unpack swig at configure time
  configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.swig swig-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/swig-download )
  if(result)
    message(FATAL_ERROR "CMake step for swig failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/swig-download )
  if(result)
    message(FATAL_ERROR "Build step for swig failed: ${result}")
  endif()

  # Tell CMake where to find the swig executable, which then sets up the other
  # cmake variables needed for include and link libraries.
  if(WIN32)
    set(SWIG_EXECUTABLE ${CMAKE_BINARY_DIR}/swig/swig.exe)
  else()
    set(SWIG_EXECUTABLE ${CMAKE_BINARY_DIR}/swig/install/bin/swig)
  endif()
  message(STATUS "Getting SWIG: ...DONE")
endif()

if (SWAPPED_ARGS_BUILD_PYTHON)
  add_subdirectory(swig)
endif()

option(SWAPPED_ARGS_BUILD_TESTS "Build tests." ON)
if(SWAPPED_ARGS_BUILD_TESTS)
  message(STATUS "Getting googletest: ...")
  # Download and unpack googletest at configure time
  configure_file(CMakeLists.googletest googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()
  message(STATUS "Getting googletest: ...DONE")

  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                   ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                   EXCLUDE_FROM_ALL)
  
  add_subdirectory(test)
endif()