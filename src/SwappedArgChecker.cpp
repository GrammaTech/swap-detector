#include "SwappedArgChecker.hpp"
#include "IdentifierSplitting.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace swapped_arg;

struct MorphemeSet {
  std::set<std::string> Morphemes;
  size_t Position;
};

std::string Result::debugStr() const { return ""; }

void Checker::CheckSite(const CallSite& site,
                        std::function<void(const Result&)> reportCallback) {
  // Walk through each combination of argument pairs from the call side.
  const std::vector<CallSite::ArgumentNames>& args = site.positionalArgNames;
  const CallDeclDescriptor& decl = site.callDecl;
  std::vector<std::pair<size_t, size_t>> argPairs =
      pairwise_combinations(args.size());
  for (const auto& pairwiseArgs : argPairs) {
    // If there is a corresponding parameter for both arguments, we may be
    // able to run the cover-based checker. Consider:
    // void foo(int i, ...); foo(1, 2, 3, 4);
    // as an example of when an argument may not have a corresponding parameter.
    // Also, check that if we have a parameter for an argument, that the
    // parameter has a name. Consider:
    // void foo(int i, int, int, int j); foo(1, 2, 3, 4);
    // as an example of when an argument may not have a corresponding named
    // parameter.
    if (decl.paramNames && pairwiseArgs.first < decl.paramNames->size() &&
        pairwiseArgs.second < decl.paramNames->size() &&
        !decl.paramNames->at(pairwiseArgs.first).empty() &&
        !decl.paramNames->at(pairwiseArgs.second).empty()) {
      // Having verified we might be able to run the cover-based checker, now
      // split the parmeter identifiers into individual morphemes and verify
      // that we have at least one usable morpheme for both parameters. Split
      // into a set so that the morphemes must be unique.
      IdentifierSplitter splitter;
      MorphemeSet param1Morphemes{
          splitter.split(decl.paramNames->at(pairwiseArgs.first)),
          pairwiseArgs.first + 1},
          param2Morphemes{
              splitter.split(decl.paramNames->at(pairwiseArgs.second)),
              pairwiseArgs.second + 1};
      if (param1Morphemes.Morphemes.empty() ||
          param2Morphemes.Morphemes.empty())
        continue;

      // Do the same thing for arguments, except all argument components are
      // split into the same set. e.g., foo(bar_baz + quux_baz, 0) would split
      // the first argument into the set [bar, baz, quux]. Verify there is at
      // least one usable morpheme for both arguments.
      MorphemeSet arg1Morphemes, arg2Morphemes;
      arg1Morphemes.Position = pairwiseArgs.first + 1;
      for (const auto& arg : args[pairwiseArgs.first]) {
        const auto& morphs = splitter.split(arg);
        arg1Morphemes.Morphemes.insert(morphs.begin(), morphs.end());
      }
      arg2Morphemes.Position = pairwiseArgs.second + 1;
      for (const auto& arg : args[pairwiseArgs.second]) {
        const auto& morphs = splitter.split(arg);
        arg2Morphemes.Morphemes.insert(morphs.begin(), morphs.end());
      }
      if (arg1Morphemes.Morphemes.empty() || arg2Morphemes.Morphemes.empty())
        continue;

      runCoverBasedChecker(std::make_pair(param1Morphemes, param2Morphemes),
                           std::make_pair(arg1Morphemes, arg2Morphemes),
                           reportCallback);
    }
  }
}
