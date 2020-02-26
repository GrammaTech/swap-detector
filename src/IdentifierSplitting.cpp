#include "IdentifierSplitting.hpp"
#include <cctype>

std::vector<std::string>
IdentifierSplitter::split(const std::string& Input) const {
  // This is a rudimentary implementation that splits only on transition from
  // lowercase to uppercase, or when finding a hard word boundary like _.
  // This does not do anything special to handle double underscores, leading
  // or trailing underscores, etc. It's just a placeholder for testing.
  // FIXME: use of islower() and isupper() depends on the current C locale.

  std::vector<std::string> Ret;
  const char* WordStart = Input.data();
  bool PrevCharWasLower = std::islower(*WordStart);
  for (const char *CurLoc = WordStart, *End = WordStart + Input.length();
       CurLoc < End; ++CurLoc) {
    if (*CurLoc == '_') {
      // We've ended the word. Add only if we have an actual word, which
      // handles duplicate underscores.
      if (WordStart != CurLoc) {
        Ret.push_back(std::string(WordStart, CurLoc));
      }
      WordStart = CurLoc + 1; // Advance past the _.
    } else if (std::isupper(*CurLoc) && PrevCharWasLower) {
      // Transitions from lowercase to uppercase are treated as a word boundary.
      Ret.push_back(std::string(WordStart, CurLoc));
      WordStart = CurLoc; // Start at the capital letter.
    }
    PrevCharWasLower = std::islower(*CurLoc);
  }

  // Add the last part of the string, if any, to the splits.
  if (WordStart != Input.data() + Input.length()) {
    Ret.push_back(std::string(WordStart, Input.data() + Input.length()));
  }

  return Ret;
}
