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

// Defines the Clang static analyzer plugin interfaces for two checkers:
// gt.SwapDetector
//   Detects argument swaps at a call site based on either the names of the
//   parameters for the function being calls, or based on statistical
//   information gathered about how the function is typically called.
//
//   This checker has a ModelPath configuration option used to specify the path
//   to the statistics database to be used by the tool. Defaults to not using a
//   statistics database.
//
// gt.ExprNames
//    Used to help test the expression name extraction functionality and is not
//    likely to be useful in other contexts.
//
// Usage: scan-build -load-plugin SwapDetectorPlugin.so
//                   -enable-checker gt.SwapDetector
//                   -analyzer-config gt.SwapDetector:ModelPath=sample.db
//        clang <options> source_file

#include "ExprNamesInspectionChecker.hpp"
#include "SwappedArgCheckerPlugin.hpp"
#include <clang/StaticAnalyzer/Core/AnalyzerOptions.h>
#include <clang/StaticAnalyzer/Frontend/CheckerRegistry.h>

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

static void initializeSwappedArgChecker(CheckerManager &mgr) {
  StringRef modelPath = mgr.getAnalyzerOptions().getCheckerStringOption(
      "gt.SwapDetector", "ModelPath");
  (void)mgr.registerChecker<SwappedArgChecker>(modelPath.str());
}


static bool alwaysRegister(const LangOptions &) { return true; }

extern "C" void clang_registerCheckers(CheckerRegistry &registry) {
  registry.addCheckerOption("string", "gt.SwapDetector", "ModelPath", "", "",
                            "alpha");
  registry.addChecker(&initializeSwappedArgChecker, &alwaysRegister,
                      "gt.SwapDetector", "Check for swapped arguments", "",
                      false);
  registry.addChecker<ExprNamesInspectionChecker>(
      "gt.ExprNames", "Print expr names for regression tests", "");
}

