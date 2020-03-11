set(PROJECT_NAME TestSwappedArgsCpp)

enable_testing()
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${SWAPPED_ARG_INCLUDE_DIR})

add_compile_options(-DDEBUG)

set(${PROJECT_NAME}_H)

set(${PROJECT_NAME}_SRC
    Foo.test.cpp
    IdentifierSplitting.test.cpp
    main.cpp
)

if(UNIX AND NOT WIN32)
  set(SYSLIBS dl)
else()
  set(SYSLIBS)
endif()

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/cpp")

target_link_libraries(
  ${PROJECT_NAME} ${SYSLIBS} gtest gmock SwappedArgChecker
)
