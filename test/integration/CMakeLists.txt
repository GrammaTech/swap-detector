set(PROJECT_NAME IntegrationTestSwappedArgs)

set(${PROJECT_NAME}_H)

set(${PROJECT_NAME}_SRC
    main.cpp
)

include_directories(${SWAPPED_ARG_INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/integration")

target_link_libraries(
  ${PROJECT_NAME} ${SYSLIBS} SwapDetector
)

set(PY_SOURCES
  integration_test_runner.py
)

set(PY_SIDEFILES
  names_subset.json
)

add_custom_target(pyIntegrationTester ALL DEPENDS ${PY_SOURCES} ${PY_SIDEFILES})
add_custom_command(TARGET pyIntegrationTester
  COMMAND ${CMAKE_COMMAND} -E copy ${PY_SOURCES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${PY_SIDEFILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_dependencies(${PROJECT_NAME} pyIntegrationTester)
