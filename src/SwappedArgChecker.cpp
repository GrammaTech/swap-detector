#include "SwappedArgChecker.hpp"
#include "IdentifierSplitting.hpp"
#include <algorithm>
#include <cassert>
#include <experimental/iterator>
#include <iostream>
#include <iterator>
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

void Checker::print(const MorphemeSet& m, bool isArg) {
  std::cout << (isArg ? "argument" : "parameter") << " MorphemeSet ("
            << m.Position << "): ";
  std::copy(m.Morphemes.begin(), m.Morphemes.end(),
            std::experimental::make_ostream_joiner(std::cout, ", "));
  std::cout << std::endl;
}

// Returns true if the checker reported any issues; false otherwise.
std::optional<Result> Checker::checkForCoverBasedSwap(
    const std::pair<MorphemeSet, MorphemeSet>& params,
    const std::pair<MorphemeSet, MorphemeSet>& args, const CallSite& site) {
  // We have already verified that the morpheme sets are not empty, but we
  // also need to verify that the number of morphemes is the same between each
  // parameter and argument.
  // FIXME: Roger thinks that we may relax the requirement that the number of
  // morphemes are the same between params and args, but isn't 100% certain yet.
  // If that happens, we will have to figure out what to do with the "extra"
  // morphemes in terms of scoring coverage.
  const std::set<std::string>&param1Morphs = params.first.Morphemes,
        &param2Morphs = params.second.Morphemes;
  const std::set<std::string>&arg1Morphs = args.first.Morphemes,
        &arg2Morphs = args.second.Morphemes;
  assert(!param1Morphs.empty() && !param2Morphs.empty() &&
         !arg1Morphs.empty() && !arg2Morphs.empty());

  if (param1Morphs.size() != param2Morphs.size() ||
      arg1Morphs.size() != arg2Morphs.size() ||
      param1Morphs.size() != arg1Morphs.size())
    return std::nullopt;

  // Remove any low entropy or duplicate param morphemes.
  std::set<std::string> uniqueMorphsParam1 =
                            nonLowEntropyDifference(param1Morphs, param2Morphs),
                        uniqueMorphsParam2 =
                            nonLowEntropyDifference(param2Morphs, param1Morphs);
  std::set<std::string> uniqueMorphsArg1 =
                            nonLowEntropyDifference(arg1Morphs, arg2Morphs),
                        uniqueMorphsArg2 =
                            nonLowEntropyDifference(arg2Morphs, arg1Morphs);

  // If there are not enough morphemes left after uniquing, then bail out.
  if (uniqueMorphsParam1.empty() || uniqueMorphsParam2.empty() ||
      uniqueMorphsArg1.empty() || uniqueMorphsArg2.empty())
    return std::nullopt;

  // If the morphemes seem at all good in their current locations, bail out.
  float mm_ai_pi;
  if ((mm_ai_pi = morphemesMatch(uniqueMorphsArg1, uniqueMorphsParam1,
                                 Bias::Optimistic)) >
      Opts.ExistingMorphemeMatchMax)
    return std::nullopt;
  float mm_aj_pj;
  if ((mm_aj_pj = morphemesMatch(uniqueMorphsArg2, uniqueMorphsParam2,
                                 Bias::Optimistic)) >
      Opts.ExistingMorphemeMatchMax)
    return std::nullopt;

  // If the morphemes seem at all bad when you swap them, bail out.
  float mm_ai_pj;
  if ((mm_ai_pj = morphemesMatch(uniqueMorphsArg1, uniqueMorphsParam2,
                                 Bias::Pessimistic)) <
      Opts.SwappedMorphemeMatchMin)
    return std::nullopt;
  float mm_aj_pi;
  if ((mm_aj_pi = morphemesMatch(uniqueMorphsArg2, uniqueMorphsParam1,
                                 Bias::Pessimistic)) <
      Opts.SwappedMorphemeMatchMin)
    return std::nullopt;

  // If we got here but there are numeric suffixes on the arguments or the
  // parameters, filter those out to reduce false positives.
  auto suffixCheck = [](const std::string& one, const std::string& two) {
    assert(!one.empty() && !one.empty() &&
           "Should not have empty names by this point");
    char suf1 = *(one.end() - 1), suf2 = *(one.end() - 1);
    return std::isdigit(suf1) && std::isdigit(suf2) &&
           one.substr(0, one.length() - 1) == two.substr(0, two.length() - 1);
  };
  std::string param1 = *getParamName(site, params.first.Position - 1),
              param2 = *getParamName(site, params.second.Position - 1);
  if (suffixCheck(param1, param2))
    return std::nullopt;
  std::string arg1 = *getLastArgName(site, args.first.Position - 1),
              arg2 = *getLastArgName(site, args.second.Position - 1);
  if (suffixCheck(arg1, arg2))
    return std::nullopt;

  float psi_i = mm_ai_pj / (mm_aj_pj + 0.01f),
        psi_j = mm_aj_pi / (mm_ai_pi + 0.01f);
  float worst_psi = std::min(psi_i, psi_j);
  // TODO: When adding the stats-based checker, this should become non-const
  // and be set to true if we attempted to run the stats-based checker.
  const bool verified_with_stats = false;

  Result r;
  r.arg1 = args.first.Position;
  r.arg2 = args.second.Position;
  r.score = std::make_unique<ParameterNameBasedScoreCard>(worst_psi,
                                                          verified_with_stats);
  r.morphemes1 = uniqueMorphsArg1;
  r.morphemes2 = uniqueMorphsArg2;
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 7
  // Hack around a GCC 7.x bug where the presence of a move-only data member
  // causes the std::optional constructor to be removed from consideration.
  // This is the only version of GCC we have to worry about (we don't support
  // older versions and the bug was fixed in newer versions).
  return std::move(r);
#else
  return r;
#endif
}

float Checker::anyAreSynonyms(
    const std::string& morpheme,
    const std::set<std::string>& potentialSynonyms) const {
  // FIXME: this is a very basic implementation currently.
  for (const std::string& synonym : potentialSynonyms) {
    if (synonym == morpheme) {
      return 1.0f;
    }
  }
  return 0.0f;
}

std::set<std::string>
Checker::nonLowEntropyDifference(const std::set<std::string>& lhs,
                                 const std::set<std::string>& rhs) const {
  std::set<std::string> ret;
  // FIXME: consider whether we want to handle low-entropy morphemes. For now,
  // do a set difference.
  std::set_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                      std::inserter(ret, ret.begin()));
  return ret;
}

float Checker::morphemesMatch(const std::set<std::string>& arg,
                              const std::set<std::string>& param,
                              Bias bias) const {
  BiasComp comp(bias, Opts);
  std::optional<float> extreme;
  for (const std::string& paramMorph : param) {
    float val = anyAreSynonyms(paramMorph, arg);
    if (!extreme || comp(val, *extreme)) {
      extreme = val;
    }
  }
  assert(extreme && "Expected to find an extreme");
  return *extreme;
}

// Removes low-quality morphemes from the given set. Returns true if removing
// the morphemes leaves the set empty, false otherwise.
static bool removeLowQualityMorphemes(std::set<std::string>& morphemes) {
  // FIXME: implement the reduction heuristics here.
  return morphemes.empty();
}

std::vector<Result> Checker::CheckSite(const CallSite& site) {
  // If there aren't at least two arguments to the call, there's no swapping
  // possible, so bail out early.
  const std::vector<CallSite::ArgumentNames>& args = site.positionalArgNames;
  if (args.size() < 2)
    return {};

  // Walk through each combination of argument pairs from the call site.
  const CallDeclDescriptor& decl = site.callDecl;
  std::vector<Result> results;
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
    std::string param1, param2;
    if (std::optional<std::string> n = getParamName(site, pairwiseArgs.first))
      param1 = *n;
    if (std::optional<std::string> n = getParamName(site, pairwiseArgs.second))
      param2 = *n;

    if (!param1.empty() && !param2.empty()) {
      // Having verified we might be able to run the cover-based checker, now
      // split the parameter identifiers into individual morphemes and verify
      // that we have at least one usable morpheme for each parameter.
      // FIXME: currently, the stub for IdentifierSplitter has no state and
      // requires no parameterization. If that continues to be true after
      // adding the real implementation, this should be replaced with a free
      // function. If it does have state, this may also be more natural as a
      // data member rather than a local.
      IdentifierSplitter splitter;
      MorphemeSet param1Morphemes{splitter.split(param1),
                                  pairwiseArgs.first + 1},
          param2Morphemes{splitter.split(param2), pairwiseArgs.second + 1};

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
      // morpheme for each argument.
      // FIXME: Currently, the first argument will not produce any morphemes
      // because we've not decided to stick with this approach. If we continue
      // to produce only one identifier per argument, consider flattening the
      // interface of how we represent arguments.
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
      if (std::optional<Result> coverWarning = checkForCoverBasedSwap(
              std::make_pair(param1Morphemes, param2Morphemes),
              std::make_pair(arg1Morphemes, arg2Morphemes), site)) {
        results.push_back(std::move(*coverWarning));
        continue;
      }
    }
  }

  return results;
}
