# Set up swig
find_package(SWIG REQUIRED)
include(UseSWIG)

# Add subdirectories for each language if desired
if(SWAPPED_ARGS_BUILD_PYTHON)
    add_subdirectory(python)
endif()

