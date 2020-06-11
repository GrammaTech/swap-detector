//
// Created by Joseph Ranieri on 2018-11-06.
//

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
