set(PROJECT_NAME SwappedArgChecker)

# specify header files
set(${PROJECT_NAME}_H
    "${SWAPPED_ARG_INCLUDE_DIR}/Compiler.hpp"
    "${SWAPPED_ARG_INCLUDE_DIR}/IdentifierSplitting.hpp"
    "${SWAPPED_ARG_INCLUDE_DIR}/SwappedArgChecker.hpp"
)

# specify source files
set(${PROJECT_NAME}_SRC
    IdentifierSplitting.cpp
    SwappedArgChecker.cpp
)

add_library(
  ${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "SwappedArgChecker")

# Find our headers
include_directories(${SWAPPED_ARG_INCLUDE_DIR})

