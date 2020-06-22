//===- ExprNamesInspectionCheck.cpp -----------------------------*- C++ -*-===//
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
#include "ExprNamesInspectionChecker.hpp"
#include "ExprNames.hpp"
#include <clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h>

using namespace clang;
using namespace ento;

bool ExprNamesInspectionChecker::evalCall(const CallEvent& Call,
                                          CheckerContext& C) const {
  const auto *CE = dyn_cast_or_null<CallExpr>(Call.getOriginExpr());

  if (CE && C.getCalleeName(CE) == "clang_analyzer_exprName") {
    std::string name =
        exprName(CE, CE->getArg(0), C.getSourceManager(), C.getLangOpts());
    if (!name.empty())
        reportBug(name, C);
    return true;
  }

  return false;
}

ExplodedNode* ExprNamesInspectionChecker::reportBug(llvm::StringRef Msg,
                                                    CheckerContext& C) const {
  ExplodedNode *N = C.generateNonFatalErrorNode();
  if (!N)
    return nullptr;

  if (!BT)
    BT.reset(new BugType(this, "Checking analyzer assumptions", "debug"));

  auto R = std::make_unique<PathSensitiveBugReport>(*BT, Msg, N);
  C.getBugReporter().emitReport(std::move(R));
  return N;
}
