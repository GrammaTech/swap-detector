//==- ExprInspectionChecker.cpp - Used for regression tests ------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ExprNamesInspectionChecker.hpp"
#include "ExprNames.hpp"
#include <clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h>

using namespace clang;
using namespace ento;

bool ExprInspectionChecker::evalCall(const CallEvent &Call,
                                     CheckerContext &C) const {
  const auto *CE = dyn_cast_or_null<CallExpr>(Call.getOriginExpr());

  if (CE && C.getCalleeName(CE) == "clang_analyzer_exprName") {
    std::string name = exprName(CE, CE->getArg(0), C.getSourceManager(), C.getLangOpts());
    if (!name.empty())
        reportBug(name, C);
    return true;
  }

  return false;
}

ExplodedNode *ExprInspectionChecker::reportBug(llvm::StringRef Msg,
                                               CheckerContext &C) const {
  ExplodedNode *N = C.generateNonFatalErrorNode();
  if (!N)
    return nullptr;

  if (!BT)
    BT.reset(new BugType(this, "Checking analyzer assumptions", "debug"));

  auto R = std::make_unique<PathSensitiveBugReport>(*BT, Msg, N);
  C.getBugReporter().emitReport(std::move(R));
  return N;
}
