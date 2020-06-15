//===- Plugin.cpp -----------------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2020 GrammaTech, Inc.
//
//  This code is licensed under the MIT license. See the LICENSE file in the
//  project root for license terms.
//
// This material is based on research sponsored by the Department of Homeland
// Security (DHS) Office of Procurement Operations, S&T acquisition Division via
// contract number 70RSAT19C00000056. The views and conclusions contained herein
// are those of the authors and should not be interpreted as necessarily
// representing the official policies or endorsements, either expressed or
// implied, of the Department of Homeland Security.
//
//===----------------------------------------------------------------------===//

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

#include "SwappedArgCheckerPlugin.hpp"
#include <clang/StaticAnalyzer/Core/AnalyzerOptions.h>
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

