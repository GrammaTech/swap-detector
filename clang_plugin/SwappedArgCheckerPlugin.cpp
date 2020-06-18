//===- SwappedArgCheckerPlugin.cpp ------------------------------*- C++ -*-===//
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
#include "SwappedArgCheckerPlugin.hpp"
#include "ExprNames.hpp"
#include <clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h>
#include <experimental/iterator>
#include <iterator>
#include <llvm/ADT/STLExtras.h>
#include <sstream>

static std::vector<std::string> getParamNames(const FunctionDecl *FD) {
  std::vector<std::string> Ret;
  llvm::transform(FD->parameters(), std::back_inserter(Ret),
                  [](const ParmVarDecl *PVD) {
    return PVD->getName().str();
  });
  return Ret;
}

static std::vector<swapped_arg::CallSite::ArgumentNames>
getArgNames(const CallEvent &Call, CheckerContext &Ctx) {
  std::vector<swapped_arg::CallSite::ArgumentNames> Ret;
  for (unsigned Idx = 0; Idx < Call.getNumArgs(); ++Idx) {
    if (const auto *E = Call.getArgExpr(Idx)) {
      Ret.push_back({exprName(Call.getOriginExpr(), E, Ctx.getSourceManager(),
                              Ctx.getLangOpts())});
     } else {
       Ret.push_back({});
     }
  }
  return Ret;
}

static std::string flattenMorphemeListForDiag(const std::set<std::string> &M) {
  std::ostringstream OS;
  std::copy(M.begin(), M.end(),
            std::experimental::make_ostream_joiner(OS, ", "));
  return OS.str();
}

void SwappedArgChecker::checkPreCall(const CallEvent &Call,
                                     CheckerContext &C) const {
  auto *FD = dyn_cast_or_null<FunctionDecl>(Call.getDecl());
  if (!FD)
    return;

  using namespace swapped_arg;
  CallDeclDescriptor CDD;
  CDD.fullyQualifiedName = FD->getQualifiedNameAsString();
  CDD.paramNames = getParamNames(FD);

  CallSite CS;
  CS.callDecl = CDD;
  CS.positionalArgNames = getArgNames(Call, C);

  std::vector<Result> Results = Check.CheckSite(CS);
  for (const auto &R : Results) {
    std::string Msg =
        ("arguments " + Twine(R.arg1) + " and " + Twine(R.arg2) +
         " are swapped with morpheme1 = " +
         flattenMorphemeListForDiag(R.morphemes1) +
         " and morpheme2 = " + flattenMorphemeListForDiag(R.morphemes2))
            .str();
    // Expects zero-based argument indexes, hence the -1.
    reportRuleViolation(Call, C, R.arg1 - 1, R.arg2 - 1, Msg);
  }
}

void SwappedArgChecker::reportRuleViolation(const CallEvent &Call,
                                            CheckerContext &C,
                                            size_t Arg1, size_t Arg2,
                                            llvm::StringRef Message) const {
  ExplodedNode *N = C.generateNonFatalErrorNode(C.getState());
  if (N) {
    if (!BT) {
      BT.reset(new BugType(this, "swapped arguments", "API", true));
    }
    auto R = std::make_unique<PathSensitiveBugReport>(*BT, Message, N);
    R->addRange(Call.getArgSourceRange(Arg1));
    R->addRange(Call.getArgSourceRange(Arg2));
    if (auto *E = Call.getArgExpr(Arg1))
      bugreporter::trackExpressionValue(N, E, *R);
    if (auto *E = Call.getArgExpr(Arg2))
      bugreporter::trackExpressionValue(N, E, *R);
    C.emitReport(std::move(R));
  }
}

SwappedArgChecker::SwappedArgChecker(const std::string &modelPath)
    : Check({modelPath}) {}
