set(PROJECT_NAME SwapDetector)

# specify header files
set(${PROJECT_NAME}_H
    "${SWAPPED_ARG_INCLUDE_DIR}/IdentifierSplitting.hpp"
    "${SWAPPED_ARG_INCLUDE_DIR}/SwappedArgChecker.hpp"
    "sqlite3.h"
    "sqlite3ext.h"
)

# specify source files
set(${PROJECT_NAME}_SRC
    IdentifierSplitting.cpp
    SwappedArgChecker.cpp
    sqlite3.c
)

add_library(
  ${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC}
)

if(UNIX)
  # Link against libdl because sqlite3 requires it.
  target_link_libraries(${PROJECT_NAME} dl)

  # Also needs to support pthreads.
  set(THREAD_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME} Threads::Threads)
elseif (MSVC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "SwapDetector")

# Find our headers
include_directories(${SWAPPED_ARG_INCLUDE_DIR})

