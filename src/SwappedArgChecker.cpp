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

// Calculates the indicies for all the pair-wise combinations from a list
// of totalCount length.
static std::vector<std::pair<size_t, size_t>>
pairwise_combinations(size_t totalCount) {
  std::vector<std::pair<size_t, size_t>> ret;
  std::vector<bool> bitset(2, true);
  bitset.resize(totalCount, false);

  do {
    size_t first = ~0U, second = ~0U;
    for (size_t idx = 0; idx < totalCount; ++idx) {
      if (bitset[idx]) {
        if (first == ~0U)
          first = idx;
        else {
          assert(second == ~0U);
          second = idx;
        }
      }
    }
    assert(first != ~0U && second != ~0U);
    ret.push_back(std::make_pair(first, second));
  } while (std::prev_permutation(bitset.begin(), bitset.end()));
  return ret;
}

// Returns true if the checker reported any issues; false otherwise.
static bool
runCoverBasedChecker(const std::pair<MorphemeSet, MorphemeSet>& params,
                     const std::pair<MorphemeSet, MorphemeSet>& args,
                     std::function<void(const Result&)> reportCallback) {
  // Randomly decide to fail for the given params and args, just assume the
  // first morpheme is what caused the problem when reporting. This is
  // placeholder code for the actual implementation.
  std::random_device rd;
  std::mt19937 gen(rd());
  if (std::generate_canonical<double, 32>(gen) < .25) {
    Result r;
    r.arg1 = args.first.Position;
    r.arg2 = args.second.Position;
    r.score = new ParameterNameBasedScoreCard(
                std::generate_canonical<double, 32>(gen) * 100.0);
    r.morpheme1 = *args.first.Morphemes.begin();
    r.morpheme2 = *args.second.Morphemes.begin();
    reportCallback(r);
    return true;
  }
  return false;
}

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

      // FIXME: run the statistics-based checker if the cover-based checker
      // does not find any issues.
      runCoverBasedChecker(std::make_pair(param1Morphemes, param2Morphemes),
                           std::make_pair(arg1Morphemes, arg2Morphemes),
                           reportCallback);
    }
  }
}
