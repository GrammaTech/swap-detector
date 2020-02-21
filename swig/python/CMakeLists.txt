set(PYTHON_PROJECT_NAME pySwappedArgChecker)

find_package(PythonLibs REQUIRED)

set(CMAKE_SWIG_FLAGS "")
set_source_files_properties(swapped_arg_checker.i PROPERTIES
                            CPLUSPLUS ON
                            SWIG_MODULE_NAME ${PYTHON_PROJECT_NAME})

# Add swig module
swig_add_library(${PYTHON_PROJECT_NAME}
                 TYPE USE_BUILD_SHARED_LIBS
                 LANGUAGE python
                 OUTPUT_DIR ${CMAKE_BINARY_DIR}/python/${PROJECT_NAME}
                 SOURCES swapped_arg_checker.i)

target_include_directories(${PYTHON_PROJECT_NAME}
	PRIVATE
	../../include
	${PYTHON_INCLUDE_DIRS}
	)
set_property(TARGET ${PYTHON_PROJECT_NAME} PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
target_link_libraries(${PYTHON_PROJECT_NAME} PRIVATE ${PROJECT_NAME} ${PYTHON_LIBRARY_RELEASE})

