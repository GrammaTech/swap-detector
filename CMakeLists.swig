cmake_minimum_required(VERSION 3.5)

project(swig-download NONE)

include(ExternalProject)
if (WIN32)
  ExternalProject_Add(SWIG_project
    URL "http://prdownloads.sourceforge.net/swig/swigwin-4.0.1.zip"
    BUILD_IN_SOURCE 1
    SOURCE_DIR "${CMAKE_BINARY_DIR}/swig"
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
  )
else()
  ExternalProject_Add(SWIG_project
    URL "http://prdownloads.sourceforge.net/swig/swig-4.0.1.tar.gz"
    BUILD_IN_SOURCE 1
    SOURCE_DIR "${CMAKE_BINARY_DIR}/swig"
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./autogen.sh
    COMMAND ./configure
    BUILD_COMMAND "make"
    INSTALL_COMMAND ""
    TEST_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
  )
endif()
