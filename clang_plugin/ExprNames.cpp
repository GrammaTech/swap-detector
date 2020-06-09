//
// Created by Joseph Ranieri on 2018-11-06.
//

#include "ExprNames.h"
#include <clang/Lex/Lexer.h>
using namespace clang;

/// Ignores any casts, parenthesis, and temporary expressions but not template
/// parameter values.
static const Expr *ignoreParenCastsButNotTemplateParms(const Expr *E) {
  while (true) {
    E = E->IgnoreParens();
    if (auto *cast = dyn_cast<CastExpr>(E)) {
      E = cast->getSubExpr();
      continue;
    }
    if (auto *materialize = dyn_cast<MaterializeTemporaryExpr>(E)) {
      E = materialize->getSubExpr();
      continue;
    }
    return E;
  }
}

std::string exprName(const Expr *CE, const Expr *expr, const SourceManager &SM,
                     const LangOptions &langOpts) {
  auto baseExpr = ignoreParenCastsButNotTemplateParms(expr);

  // If the expression is from a macro expansion, return just the macro name.
  // This turns out to be tricky.
  //
  // TODO: This is probably more complicated than it needs to be and at the
  //       very least needs commenting.
  auto exprRange = baseExpr->getSourceRange();
  if (exprRange.getBegin().isMacroID()) {
    auto expandedRange = exprRange;
    while (CE && SM.getExpansionLoc(expandedRange.getBegin()) ==
                     SM.getExpansionLoc(CE->getBeginLoc())) {
      expandedRange.setBegin(
          SM.getImmediateSpellingLoc(expandedRange.getBegin()));
      expandedRange.setEnd(SM.getImmediateSpellingLoc(expandedRange.getEnd()));
    }
    if (expandedRange.getBegin().isMacroID()) {
      expandedRange.setBegin(SM.getExpansionLoc(expandedRange.getBegin()));
      expandedRange.setEnd(SM.getExpansionLoc(expandedRange.getEnd()));
      if (expandedRange.getBegin() == expandedRange.getEnd()) {
        SmallString<32> buf;
        return Lexer::getSpelling(expandedRange.getBegin(), buf, SM, langOpts);
      }
    }
  }

  if (auto DR = dyn_cast<DeclRefExpr>(baseExpr)) {
    // If n is an identifier, return its name.
    return DR->getDecl()->getName();
  } else if (isa<IntegerLiteral>(baseExpr) || isa<CharacterLiteral>(baseExpr) ||
             isa<FloatingLiteral>(baseExpr) || isa<StringLiteral>(baseExpr) ||
             isa<ImaginaryLiteral>(baseExpr) ||
             isa<FixedPointLiteral>(baseExpr) ||
             isa<CXXBoolLiteralExpr>(baseExpr) ||
             isa<CXXNullPtrLiteralExpr>(baseExpr) ||
             isa<ObjCBoolLiteralExpr>(baseExpr)) {
    // If n is a literal, return a string representation of its value.
    std::string result;
    llvm::raw_string_ostream stream(result);
    baseExpr->printPretty(stream, nullptr, langOpts);
    return "LIT:" + stream.str();
  } else if (isa<CXXThisExpr>(baseExpr)) {
    // If n is an update expression that increments or decrements x, return
    // name(x).
    return "this";
  } else if (auto ME = dyn_cast<MemberExpr>(baseExpr)) {
    // If n is a member expression base.prop that accesses a property, return
    // name(prop).
    return ME->getMemberNameInfo().getAsString();
  } else if (auto ASE = dyn_cast<ArraySubscriptExpr>(baseExpr)) {
    // If n is a member expression base[k] that accesses an array element,
    // return name(base).
    return exprName(CE, ASE->getBase(), SM, langOpts);
  } else if (auto ArgCE = dyn_cast<CallExpr>(baseExpr)) {
    // If n is a call expression base.callee(..), return name(callee).
    if (auto ND = dyn_cast_or_null<NamedDecl>(ArgCE->getCalleeDecl())) {
      return ND->getDeclName().getAsString();
    } else if (auto callee = ArgCE->getCallee()) {
      return exprName(CE, callee, SM, langOpts);
    }
  } else if (auto UO = dyn_cast<UnaryOperator>(baseExpr)) {
    return exprName(CE, UO->getSubExpr(), SM, langOpts);
  } else if (auto UE = dyn_cast<UnaryExprOrTypeTraitExpr>(baseExpr)) {
    if (UE->isArgumentType()) {
      return UE->getTypeOfArgument().getAsString();
    } else {
      return exprName(CE, UE->getArgumentExpr(), SM, langOpts);
    }
  } else if (auto NTTP = dyn_cast<SubstNonTypeTemplateParmExpr>(baseExpr)) {
    return NTTP->getParameter()->getDeclName().getAsString();
  }

  return "";
}
