cmake_minimum_required(VERSION 3.10.0)

# Enable project versions.
cmake_policy(SET CMP0048 NEW)

if (NOT CMAKE_VERSION VERSION_LESS "3.13")
  cmake_policy(SET CMP0077 NEW)
endif()

# The version.txt file is the official record of the version number. We use the
# contents of that file to set the project version for use in other CMake files.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" ver)

string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${ver})
set(SWAPPED_ARGS_MAJOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${ver})
set(SWAPPED_ARGS_MINOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_PATCH ([0-9]*)" _ ${ver})
set(SWAPPED_ARGS_PATCH_VERSION ${CMAKE_MATCH_1})

project(
  SwapDetector
  VERSION "${SWAPPED_ARGS_MAJOR_VERSION}.${SWAPPED_ARGS_MINOR_VERSION}.${SWAPPED_ARGS_PATCH_VERSION}"
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

if(UNIX AND NOT BUILD_SHARED_LIBS)
  # Find only static libraries
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  add_compile_options(-static)
endif()

if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  add_compile_options(-fPIC)
endif()

set(SWAPPED_ARG_INCLUDE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  CACHE INTERNAL ""
)
add_subdirectory(src)

option(SWAPPED_ARGS_BUILD_PYTHON "Build python bindings." OFF)

if(SWAPPED_ARGS_BUILD_PYTHON)
  option(SWAPPED_ARGS_INSTALL_PYTHON "Install python bindings." ON)

  add_custom_target(PythonExt ALL
                    COMMAND python3 setup.py build_ext -I
                            "${SWAPPED_ARG_INCLUDE_DIR}" -L
                            "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
                    DEPENDS SwapDetector)
  if(SWAPPED_ARGS_INSTALL_PYTHON)
    install(
      CODE
      "execute_process(COMMAND python3 setup.py install
       WORKING_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/python\")"
      )
  endif()
endif()

option(SWAPPED_ARGS_BUILD_CLANG_PLUGIN "Build the clang plugin" ON)
if (SWAPPED_ARGS_BUILD_CLANG_PLUGIN)
  add_subdirectory(clang_plugin)
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
