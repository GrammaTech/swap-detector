//===- ExprNamesInspectionChecker.cpp ---------------------------*- C++ -*-===//
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
#include <clang/StaticAnalyzer/Core/BugReporter/BugReporter.h>
#include <clang/StaticAnalyzer/Core/BugReporter/BugType.h>
#include <clang/StaticAnalyzer/Core/Checker.h>

using namespace clang;
using namespace ento;

class ExprNamesInspectionChecker : public Checker<eval::Call> {
  mutable std::unique_ptr<BugType> BT;

  ExplodedNode *reportBug(llvm::StringRef Msg, CheckerContext &C) const;

public:
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
};
