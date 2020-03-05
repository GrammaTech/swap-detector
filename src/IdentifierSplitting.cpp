#include "IdentifierSplitting.hpp"
#include <cctype>

std::vector<std::string>
IdentifierSplitter::split(const std::string& input) const {
  // This is a rudimentary implementation that splits only on transition from
  // lowercase to uppercase, or when finding a hard word boundary like _.
  // This does not do anything special to handle double underscores, leading
  // or trailing underscores, etc. It's just a placeholder for testing.
  // FIXME: use of islower() and isupper() depends on the current C locale.

  std::vector<std::string> ret;
  const char* wordStart = input.data();
  bool prevCharWasLower = std::islower(*wordStart);
  for (const char *curLoc = wordStart, *end = wordStart + input.length();
       curLoc < end; ++curLoc) {
    if (*curLoc == '_') {
      // We've ended the word. Add only if we have an actual word, which
      // handles duplicate underscores.
      if (wordStart != curLoc) {
        ret.push_back(std::string(wordStart, curLoc));
      }
      wordStart = curLoc + 1; // Advance past the _.
    } else if (std::isupper(*curLoc) && prevCharWasLower) {
      // Transitions from lowercase to uppercase are treated as a word boundary.
      ret.push_back(std::string(wordStart, curLoc));
      wordStart = curLoc; // Start at the capital letter.
    }
    prevCharWasLower = std::islower(*curLoc);
  }

  // Add the last part of the string, if any, to the splits.
  if (wordStart != input.data() + input.length()) {
    ret.push_back(std::string(wordStart, input.data() + input.length()));
  }

  return ret;
}
