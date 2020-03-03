set(PROJECT_NAME IntegrationTestSwappedArgs)

set(${PROJECT_NAME}_H)

set(${PROJECT_NAME}_SRC
    main.cpp
)

if(UNIX AND NOT WIN32)
  set(SYSLIBS dl)
else()
  set(SYSLIBS)
endif()

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_H} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/integration")

target_link_libraries(
  ${PROJECT_NAME} ${SYSLIBS} SwappedArgChecker
)

