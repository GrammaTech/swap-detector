//===- IdentifierSplitting.cpp ----------------------------------*- C++ -*-===//
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
#include "IdentifierSplitting.hpp"
#include <algorithm>
#include <cctype>

using namespace swapped_arg;

static std::string lower(const char* wordStart, const char* wordEnd) {
  std::string str(wordStart, wordEnd);
  // FIXME: use of tolower() depends on the current C locale.
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return str;
}

std::set<std::string>
IdentifierSplitter::split(const std::string& input) const {
  // This is a rudimentary implementation that splits only on transition from
  // lowercase to uppercase, or when finding a hard word boundary like _.
  // This does not do anything special to handle double underscores, leading
  // or trailing underscores, etc. It's just a placeholder for testing.
  // FIXME: use of islower() and isupper() depends on the current C locale.

  std::set<std::string> ret;
  const char* wordStart = input.data();
  bool prevCharWasLower = std::islower(*wordStart);
  for (const char *curLoc = wordStart, *end = wordStart + input.length();
       curLoc < end; ++curLoc) {
    if (*curLoc == '_') {
      // We've ended the word. Add only if we have an actual word, which
      // handles duplicate underscores.
      if (wordStart != curLoc) {
        ret.insert(lower(wordStart, curLoc));
      }
      wordStart = curLoc + 1; // Advance past the _.
    } else if (std::isupper(*curLoc) && prevCharWasLower) {
      // Transitions from lowercase to uppercase are treated as a word boundary.
      ret.insert(lower(wordStart, curLoc));
      wordStart = curLoc; // Start at the capital letter.
    }
    prevCharWasLower = std::islower(*curLoc);
  }

  // Add the last part of the string, if any, to the splits.
  if (wordStart != input.data() + input.length()) {
    ret.insert(lower(wordStart, input.data() + input.length()));
  }

  return ret;
}
