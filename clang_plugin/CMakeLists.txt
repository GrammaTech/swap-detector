find_package(LLVM 10 CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})
if(WIN32 OR CYGWIN)
    # DLL platform -- put DLLs into bin.
    set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_RUNTIME_OUTPUT_INTDIR})
else()
    set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_LIBRARY_OUTPUT_INTDIR})
endif()

include(AddLLVM)
include(HandleLLVMOptions)
include_directories(
  ${LLVM_INCLUDE_DIRS}
  "../include"
)

add_definitions(${LLVM_DEFINITIONS})

add_llvm_library(SwapDetectorPlugin MODULE
                 ExprNames.cpp
                 ExprNamesInspectionChecker.cpp
                 Plugin.cpp
                 SwappedArgCheckerPlugin.cpp
                 PLUGIN_TOOL
                 clang
                 DEPENDS
                 SwapDetector
                 LINK_LIBS
                 SwapDetector)

add_subdirectory(test)
