//===- SwappedArgCheckerPlugin.hpp ------------------------------*- C++ -*-===//
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
#ifndef PLUGIN_SWAPPEDARGCHECKERPLUGIN__H
#define PLUGIN_SWAPPEDARGCHECKERPLUGIN_H

#include <clang/StaticAnalyzer/Core/BugReporter/BugReporter.h>
#include <clang/StaticAnalyzer/Core/BugReporter/BugType.h>
#include <clang/StaticAnalyzer/Core/Checker.h>
#include "SwappedArgChecker.hpp"

using namespace clang;
using namespace clang::ento;

class SwappedArgChecker : public Checker<check::PreCall> {
  mutable swapped_arg::Checker Check;
  mutable std::unique_ptr<BugType> BT;

  friend class check::PreCall;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void reportRuleViolation(const CallEvent &Call, CheckerContext &C, size_t Arg1,
                           size_t Arg2, llvm::StringRef Message) const;

public:
  SwappedArgChecker(const std::string &modelPath);
};

#endif // PLUGIN_SWAPPEDARGCHECKERPLUGIN_H

