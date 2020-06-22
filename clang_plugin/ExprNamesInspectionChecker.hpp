//==- ExprInspectionChecker.cpp - Used for regression tests ------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <clang/StaticAnalyzer/Core/BugReporter/BugReporter.h>
#include <clang/StaticAnalyzer/Core/BugReporter/BugType.h>
#include <clang/StaticAnalyzer/Core/Checker.h>

using namespace clang;
using namespace ento;

class ExprInspectionChecker : public Checker<eval::Call> {
  mutable std::unique_ptr<BugType> BT;

  ExplodedNode *reportBug(llvm::StringRef Msg, CheckerContext &C) const;

public:
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
};
