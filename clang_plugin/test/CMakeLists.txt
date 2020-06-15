configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

add_lit_testsuite(check-all "Running the Swapped Arg regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS SwapDetectorPlugin
  )

# llvm-lit isn't installed as part of LLVM, even if you choose to install the
# utils. Provide a helpful error message if we can't find it.
if ("${LLVM_EXTERNAL_LIT}" STREQUAL "")
  get_llvm_lit_path(LIT_BASE_DIR LIT_FILE_NAME)
  if (NOT EXISTS ${LIT_BASE_DIR}/${LIT_FILE_NAME})
    message(WARN "${LIT_BASE_DIR}/${LIT_FILE_NAME} does not exist and you will"
                 " not be able to run tests. Perhaps you need to install lit"
                 " and specify LLVM_EXTERNAL_LIT?")
  endif()
endif()
