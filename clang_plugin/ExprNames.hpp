//===- ExprNames.hpp --------------------------------------------*- C++ -*-===//
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
#ifndef PROJECT_EXPRNAMES_H
#define PROJECT_EXPRNAMES_H

#include <clang/AST/AST.h>

/// Extracts a "name" from an AST node, similar to the approach in the
/// "DeepBugs" paper.
///
/// \param CE       The call expression containing this argument expression.
/// \param expr     The argument expression to name.
/// \param SM       The relevant source manager.
/// \param langOpts The translation unit's language options.
std::string exprName(const clang::Expr *CE, const clang::Expr *expr,
                     const clang::SourceManager &SM,
                     const clang::LangOptions &langOpts);

#endif //PROJECT_EXPRNAMES_H
