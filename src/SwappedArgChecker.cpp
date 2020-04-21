#include "SwappedArgChecker.hpp"
#include "IdentifierSplitting.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <utility>

using namespace swapped_arg;

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
bool Checker::checkForCoverBasedSwap(
    const std::pair<MorphemeSet, MorphemeSet>& params,
    const std::pair<MorphemeSet, MorphemeSet>& args,
    std::function<void(const Result&)> reportCallback) {
  // We have already verified that the morpheme sets are not empty, but we
  // also need to verify that the number of morphemes is the same between each
  // parameter and argument. FIXME: Roger thinks that we may relax the
  // requirement that the number of morphemes are the same between params and
  // args, but isn't 100% certain yet. If that happens, we will have to figure
  // out what to do with the "extra" morphemes in terms of scoring coverage.
  const std::set<std::string>&param1Morphs = params.first.Morphemes,
        &param2Morphs = params.second.Morphemes;
  const std::set<std::string>&arg1Morphs = args.first.Morphemes,
        &arg2Morphs = args.second.Morphemes;
  assert(!param1Morphs.empty() && !param2Morphs.empty() &&
         !arg1Morphs.empty() && !arg2Morphs.empty());

  if (param1Morphs.size() != param2Morphs.size() ||
      arg1Morphs.size() != arg2Morphs.size() ||
      param1Morphs.size() != arg1Morphs.size())
    return false;

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
    r.morpheme1 = *arg1Morphs.begin();
    r.morpheme2 = *arg2Morphs.begin();
    reportCallback(r);
    return true;
  }
  return false;
}

// Removes low-quality morphemes from the given set. Returns true if removing
// the morphemes leaves the set empty, false otherwise.
static bool removeLowQualityMorphemes(std::set<std::string>& morphemes) {
  // FIXME: implement the reduction heuristics here.
  return morphemes.empty();
}

void Checker::CheckSite(const CallSite& site,
                        std::function<void(const Result&)> reportCallback) {
  // Walk through each combination of argument pairs from the call site.
  const std::vector<CallSite::ArgumentNames>& args = site.positionalArgNames;
  const CallDeclDescriptor& decl = site.callDecl;
  std::vector<std::pair<size_t, size_t>> argPairs =
      pairwise_combinations(args.size());
  for (const auto& pairwiseArgs : argPairs) {
    // If there is a corresponding parameter for each argument, we may be
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
      // split the parameter identifiers into individual morphemes and verify
      // that we have at least one usable morpheme for each parameter.
      // FIXME: currently, the stub for IdentifierSplitter has no state and
      // requires no parameterization. If that continues to be true after
      // adding the real implementation, this should be replaced with a free
      // function. If it does have state, this may also be more natural as a
      // data member rather than a local.
      IdentifierSplitter splitter;
      const std::vector<std::string>& params = *decl.paramNames;
      MorphemeSet param1Morphemes{splitter.split(params[pairwiseArgs.first]),
                                  pairwiseArgs.first + 1},
          param2Morphemes{splitter.split(params[pairwiseArgs.second]),
                          pairwiseArgs.second + 1};

      // Having split the parameter identifiers into morphemes, remove any
      // morphemes that are low quality and bail out if there are no usable
      // morphemes left for either parameter. Consider: void foo(int i, int j);
      // as an example of when a morpheme may be of sufficiently low quality to
      // warrant ignoring it.
      if (removeLowQualityMorphemes(param1Morphemes.Morphemes) ||
          removeLowQualityMorphemes(param2Morphemes.Morphemes))
        continue;

      // Do the same thing for arguments, except all argument components are
      // split into the same set. e.g., foo(bar.baz(), 0) may split the first
      // argument into the set [bar, baz]. Verify there is at least one usable
      // morpheme for each argument. FIXME: Currently, the first argument will
      // not produce any morphemes because we've not decided to stick with this
      // approach. If we continue to produce only one identifier per argument,
      // consider flattening the interface of how we represent arguments.
      auto morphemeCollector = [&args, &splitter](MorphemeSet& m, size_t pos) {
        m.Position = pos + 1;
        for (const auto& arg : args[pos]) {
          const auto& morphs = splitter.split(arg);
          m.Morphemes.insert(morphs.begin(), morphs.end());
        }
      };

      MorphemeSet arg1Morphemes, arg2Morphemes;
      morphemeCollector(arg1Morphemes, pairwiseArgs.first);
      morphemeCollector(arg2Morphemes, pairwiseArgs.second);

      // Similar to parameters, remove any low quality morphemes from the
      // arguments and bail out if this leaves us with no usable morphemes.
      if (removeLowQualityMorphemes(arg1Morphemes.Morphemes) ||
          removeLowQualityMorphemes(arg2Morphemes.Morphemes))
        continue;

      // FIXME: run the statistics-based checker if the cover-based checker
      // does not find any issues.
      if (!checkForCoverBasedSwap(
              std::make_pair(param1Morphemes, param2Morphemes),
              std::make_pair(arg1Morphemes, arg2Morphemes), reportCallback)) {
        std::cout << "running the stats checker (someday)\n";
      }
    }
  }
}
