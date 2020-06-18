//===- IdentifierSplitting.hpp ----------------------------------*- C++ -*-===//
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
#ifndef GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H
#define GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H

#include "Compiler.hpp"
#include <set>
#include <string>

namespace swapped_arg {
class SWAPPED_ARG_EXPORT IdentifierSplitter {
public:
  // Returns a set of case-insensitive, unique morphemes for the identifier.
  // e.g., foo_barBaz_bar would result in a set [foo, bar, baz], not
  // [foo, bar, Baz, bar].
  std::set<std::string> split(const std::string& input) const;
};
} // end namespace swapped_arg

#endif // GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H
