# Set up swig
find_package(SWIG REQUIRED)
include(UseSWIG)

# Add subdirectories for each language if desired
option(BUILD_PYTHON "Build Python SWIG module" ON)
if(BUILD_PYTHON)
    add_subdirectory(python)
endif()

