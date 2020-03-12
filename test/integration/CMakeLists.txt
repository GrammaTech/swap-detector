set(PROJECT_NAME IntegrationTestSwappedArgs)

set(${PROJECT_NAME}_H)

set(${PROJECT_NAME}_SRC
    main.cpp
)

include_directories(${SWAPPED_ARG_INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/integration")

target_link_libraries(
  ${PROJECT_NAME} ${SYSLIBS} SwappedArgChecker
)

set(PY_SOURCES
  integration_test_runner.py
)

set(PY_SIDEFILES
  names_subset.json
)

add_custom_target(pyIntegrationTester ALL DEPENDS ${PY_SOURCES} ${PY_SIDEFILES})
list(TRANSFORM PY_SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)
list(TRANSFORM PY_SIDEFILES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)
add_custom_command(TARGET pyIntegrationTester
  COMMAND ${CMAKE_COMMAND} -E copy ${PY_SOURCES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${PY_SIDEFILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
)
add_dependencies(${PROJECT_NAME} pyIntegrationTester)
