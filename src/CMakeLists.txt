set(PROJECT_NAME SwappedArgChecker)

# specify header files
set(${PROJECT_NAME}_H
    "${CMAKE_SOURCE_DIR}/include/Compiler.hpp"
    "${CMAKE_SOURCE_DIR}/include/SwappedArgChecker.hpp"
)

# specify source files
set(${PROJECT_NAME}_SRC
    SwappedArgChecker.cpp
)

add_library(
  ${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "SwappedArgChecker")

# Find our headers
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

#install(
#  TARGETS ${PROJECT_NAME}
#  EXPORT gtirbTargets
#  INCLUDES
#  DESTINATION include
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION lib
#  ARCHIVE DESTINATION lib
#)
#install(FILES ${${PROJECT_NAME}_H} DESTINATION include/gtirb)
#install(DIRECTORY "${CMAKE_BINARY_DIR}/include/proto" DESTINATION include)
#install(FILES "${CMAKE_BINARY_DIR}/include/gtirb/version.h"
#        DESTINATION include/gtirb
#)

