set(PROJECT_NAME TestSwappedArgsCpp)

enable_testing()
include_directories(${GTEST_INCLUDE_DIRS})

add_compile_options(-DDEBUG)

set(${PROJECT_NAME}_H)

set(${PROJECT_NAME}_SRC
    Foo.test.cpp
)

if(UNIX AND NOT WIN32)
  set(SYSLIBS dl)
else()
  set(SYSLIBS)
endif()

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC})
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/cpp")

target_link_libraries(
  ${PROJECT_NAME} ${SYSLIBS} ${Boost_LIBRARIES} gtest gtest_main SwappedArgChecker
)

# Add tests to make test
add_test(NAME testswappedargscpp COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
