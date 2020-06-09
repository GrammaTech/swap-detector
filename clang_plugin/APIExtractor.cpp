/*
 * Extracts information about API usage.
 *
 * USAGE:
 * clang -cc1 -w -analyze -load APIExtractor.dylib -analyzer-checker=gt,alpha.security.taint foo.c 
 *
 * Further reading:
 * - http://clang-analyzer.llvm.org/checker_dev_manual.html
 * - https://github.com/haoNoQ/clang-analyzer-guide/releases/download/v0.1/clang-analyzer-guide-v0.1.pdf
 * - "A memory model for static analysis of C programs" by Xu, Kremenek, and
 *   Zhang
 * - "Precise interprocedural dataflow analysis via graph reachability" by Reps,
 *   Horwitz, and Sagiv
 */

#include "SwappedArgChecker.h"
#include <clang/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h>
#include <clang/StaticAnalyzer/Frontend/CheckerRegistry.h>

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

static void initializeSwappedArgChecker(CheckerManager &mgr) {
  StringRef modelPath = mgr.getAnalyzerOptions().getCheckerStringOption("gt.SwappedArgs",
                                                                        "ModelPath");
  (void)mgr.registerChecker<SwappedArgChecker>(modelPath.str());
}


static bool alwaysRegister(const LangOptions &) { return true; }

extern "C" void clang_registerCheckers(CheckerRegistry &registry) {
  registry.addCheckerOption("string", "gt.SwappedArgs", "ModelPath", "", "",
                            "alpha");
  registry.addChecker(&initializeSwappedArgChecker, &alwaysRegister,
                      "gt.SwappedArgs", "Check for swapped arguments", "",
                      false);
}

