//===- SwappedArgChecker.cpp ------------------------------------*- C++ -*-===//
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
#include "SwappedArgChecker.hpp"
#include "IdentifierSplitting.hpp"
#include "sqlite3.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <experimental/iterator>
#include <iostream>
#include <iterator>
#include <sstream>
#include <utility>

namespace swapped_arg {
class Statistics {
  sqlite3* db = nullptr;
  sqlite3_stmt* morph_value_query = nullptr;
  sqlite3_stmt* value_query = nullptr;

public:
  explicit Statistics(const std::string& path) {
    // We purposefully do not care about a failure to load the database at this
    // stage. The valid() method can be used to determine if the Statistics
    // object is valid or not.
    if (!path.empty() &&
        SQLITE_OK ==
            sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr)) {
      (void)sqlite3_prepare_v2(
          db,
          "SELECT morpheme, value FROM weights WHERE func == ? AND arg == ?",
          -1, &morph_value_query, nullptr);
      (void)sqlite3_prepare_v2(db,
                               "SELECT value FROM weights WHERE func == ? AND "
                               "arg == ? AND morpheme == ?",
                               -1, &value_query, nullptr);
    }
  }
  ~Statistics() {
    if (morph_value_query) {
      (void)sqlite3_finalize(morph_value_query);
    }
    if (value_query) {
      (void)sqlite3_finalize(value_query);
    }
    if (db) {
      (void)sqlite3_close(db);
    }
  }

  // Returns true if the Statistics class has a valid statistics database,
  // false otherwise.
  bool valid() const {
    return db != nullptr && morph_value_query != nullptr &&
           value_query != nullptr;
  }

  // Finds how often the given morpheme is used at the specified position for a
  // given function call. Returns 0.0f if the function does not exist or the
  // argument position is invalid.
  float weightForMorphemeAtPos(const std::string& funcName, size_t argPos,
                               const std::string& morpheme) {
    assert(valid() && "no valid database loaded");

    // Helper RAII structure which binds the query arguments to the query on
    // construction and resets the query on destruction.
    class Binder {
      sqlite3_stmt* query;

    public:
      Binder(sqlite3_stmt* stmt, const std::string& funcName, size_t argPos,
             const std::string& morpheme)
          : query(stmt) {
        (void)sqlite3_bind_text(query, 1, funcName.c_str(), -1,
                                SQLITE_TRANSIENT);
        (void)sqlite3_bind_int64(query, 2, static_cast<sqlite3_int64>(argPos));
        (void)sqlite3_bind_text(query, 3, morpheme.c_str(), -1,
                                SQLITE_TRANSIENT);
      }
      ~Binder() {
        (void)sqlite3_clear_bindings(query);
        (void)sqlite3_reset(query);
      }
    } binder(value_query, funcName, argPos, morpheme);

    for (;;) {
      int rc = sqlite3_step(value_query);
      if (rc == SQLITE_DONE) {
        break;
      } else if (rc == SQLITE_ROW) {
        return static_cast<float>(sqlite3_column_double(value_query, 0));
      } else {
        break;
      }
    }
    return 0.0f;
  }

  // Finds all morphemes for the given function call and argument position, as
  // well as the scaled weight for each morpheme. The sum of the weights at
  // that position add up to 1. Returns false if the function does not exist or
  // the argument position is invalid; true otherwise.
  bool
  morphemesAndWeightsAtPos(const std::string& funcName, size_t argPos,
                           std::vector<std::pair<std::string, float>>& res) {
    assert(valid() && "no valid database loaded");

    // Helper RAII structure which binds the query arguments to the query on
    // construction and resets the query on destruction.
    class Binder {
      sqlite3_stmt* query;

    public:
      Binder(sqlite3_stmt* stmt, const std::string& funcName, size_t argPos)
          : query(stmt) {
        (void)sqlite3_bind_text(query, 1, funcName.c_str(), -1,
                                SQLITE_TRANSIENT);
        (void)sqlite3_bind_int64(query, 2, static_cast<sqlite3_int64>(argPos));
      }
      ~Binder() {
        (void)sqlite3_clear_bindings(query);
        (void)sqlite3_reset(query);
      }
    } binder(morph_value_query, funcName, argPos);

    bool ret = false;
    for (;;) {
      int rc = sqlite3_step(morph_value_query);
      if (rc == SQLITE_DONE) {
        break;
      } else if (rc == SQLITE_ROW) {
        ret = true;
        res.emplace_back(
            std::string(reinterpret_cast<const char*>(
                sqlite3_column_text(morph_value_query, 0))),
            static_cast<float>(sqlite3_column_double(morph_value_query, 1)));
      } else {
        return false;
      }
    }
    return ret;
  }
};
} // namespace swapped_arg

using namespace swapped_arg;

std::string test::createStatsDB(std::initializer_list<test::StatsDBRow> rows) {
  std::string file_name = ::tmpnam(nullptr);

  sqlite3* db = nullptr;
  int rc = sqlite3_open(file_name.c_str(), &db);
  if (rc != SQLITE_OK)
    return "";

  rc = sqlite3_exec(db,
                    "CREATE TABLE weights ("
                    "func TEXT NOT NULL,"
                    "arg INTEGER NOT NULL CHECK(arg >= 0),"
                    "morpheme TEXT NOT NULL,"
                    "value REAL NOT NULL CHECK(value >= 0 AND value <= 1)"
                    ");",
                    nullptr, nullptr, nullptr);
  assert(rc == SQLITE_OK && "Could not create a table in the database");

  for (const auto& row : rows) {
    auto [func, arg, morpheme, value] = row;
    std::stringstream ss;
    ss << "INSERT INTO weights "
       << "('func', 'arg', 'morpheme', 'value')"
       << "VALUES ('" << func << "', '" << arg << "', '" << morpheme << "', '"
       << value << "');";
    rc = sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, nullptr);
    assert(rc == SQLITE_OK && "Could not add a row to the database");
  }

  (void)sqlite3_close(db);

  return file_name;
}

// Calculates the zero-based indicies for all the pair-wise combinations from
// a list of totalCount length.
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
  std::string param1 = *getParamName(site, params.first.Position),
              param2 = *getParamName(site, params.second.Position);
  if (suffixCheck(param1, param2))
    return std::nullopt;
  std::string arg1 = *getLastArgName(site, args.first.Position),
              arg2 = *getLastArgName(site, args.second.Position);
  if (suffixCheck(arg1, arg2))
    return std::nullopt;

  float psi_i = mm_ai_pj / (mm_aj_pj + 0.01f),
        psi_j = mm_aj_pi / (mm_ai_pi + 0.01f);
  float worst_psi = std::min(psi_i, psi_j);
  // TODO: When adding the stats-based checker, this should become non-const
  // and be set to true if we attempted to run the stats-based checker.
  const bool verified_with_stats = false;

  Result r;
  r.arg1 = args.first.Position + 1;
  r.arg2 = args.second.Position + 1;
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

Checker::MorphemeSet
Checker::morphemeSetDifference(const MorphemeSet& one,
                               const MorphemeSet& two) const {
  MorphemeSet ret;
  // We are removing the duplicates from the first set to report the difference,
  // so the position followed the first set.
  ret.Position = one.Position;
  std::set_difference(one.Morphemes.begin(), one.Morphemes.end(),
                      two.Morphemes.begin(), two.Morphemes.end(),
                      std::inserter(ret.Morphemes, ret.Morphemes.begin()));
  return ret;
}

float Checker::morphemeConfidenceAtPosition(const CallSite& callSite,
                                            const std::string& morph,
                                            size_t pos, size_t comparedToPos,
                                            Statistics& stats) const {
  float pos1 = stats.weightForMorphemeAtPos(
      callSite.callDecl.fullyQualifiedName, pos, morph);
  float pos2 = stats.weightForMorphemeAtPos(
      callSite.callDecl.fullyQualifiedName, comparedToPos, morph);

  // If there's no confidence in either position, then return 0, but if there
  // is confidence at pos1 but not at pos2, then return 1. Otherwise, return
  // the ratio pos1 to pos2.
  if (pos2 == 0.0f) {
    return pos1 != 0.0f ? 1.0f : 0.0f;
  }
  return pos1 / pos2;
}

float Checker::similarity(const std::string& morph1,
                          const std::string& morph2) const {
  // TODO: implement this
  return morph1 == morph2 ? 1.0f : 0.0f;
}

float Checker::fit(const std::string& morph, const CallSite& site,
                   size_t argPos, Statistics& stats) const {
  std::string funcName = site.callDecl.fullyQualifiedName;
  std::vector<std::pair<std::string, float>> morphsAndWeightsAtPos;
  if (!stats.morphemesAndWeightsAtPos(funcName, argPos, morphsAndWeightsAtPos))
    return 0.0f;

  float ret = 0.0f;
  for (const auto& [m, weight] : morphsAndWeightsAtPos) {
    ret += similarity(morph, m) * weight;
  }
  return ret;
}

std::optional<Result> Checker::checkForStatisticsBasedSwap(
    const std::pair<MorphemeSet, MorphemeSet>& params,
    const std::pair<MorphemeSet, MorphemeSet>& args, const CallSite& callSite,
    Statistics& stats) {
  MorphemeSet uniqArgMorphs1 = morphemeSetDifference(args.first, args.second),
              uniqArgMorphs2 = morphemeSetDifference(args.second, args.first);

  for (const std::string& argMorph1 : uniqArgMorphs1.Morphemes) {
    for (const std::string& argMorph2 : uniqArgMorphs2.Morphemes) {
      // Check to see how much more common the first morpheme is at position 2
      // than position 1, and how much more common the second morpheme is at
      // position 1 than position 2. If they seem to not be commonly swapped,
      // move on.
      float psi1 = morphemeConfidenceAtPosition(callSite, argMorph1,
                                                uniqArgMorphs2.Position,
                                                uniqArgMorphs1.Position, stats),
            psi2 = morphemeConfidenceAtPosition(callSite, argMorph2,
                                                uniqArgMorphs1.Position,
                                                uniqArgMorphs2.Position, stats);
      if (psi1 <= Opts.StatsSwappedMorphemeThreshold ||
          psi2 <= Opts.StatsSwappedMorphemeThreshold) {
        continue;
      }

      // Only consider the case where the remainder of the morphemes are the
      // same between both arguments.
      std::set<std::string> one, two;
      std::remove_copy(uniqArgMorphs1.Morphemes.begin(),
                       uniqArgMorphs1.Morphemes.end(),
                       std::inserter(one, one.begin()), argMorph1);
      std::remove_copy(uniqArgMorphs2.Morphemes.begin(),
                       uniqArgMorphs2.Morphemes.end(),
                       std::inserter(two, two.begin()), argMorph2);
      if (!std::equal(one.begin(), one.end(), two.begin(), two.end())) {
        continue;
      }

      // Determine the fitness of the first arg morpheme compared to the second
      // and vice versa to see if it exceeds a threshold.
      float fit1 = fit(argMorph1, callSite, args.second.Position, stats),
            fit2 = fit(argMorph2, callSite, args.first.Position, stats);
      if (fit1 > Opts.StatsSwappedFitnessThreshold &&
          fit2 > Opts.StatsSwappedFitnessThreshold) {
        // Return the statistical swap result.
        Result r;
        r.arg1 = args.first.Position + 1;
        r.arg2 = args.second.Position + 1;
        r.score = std::make_unique<UsageStatisticsBasedScoreCard>(fit1, fit2,
                                                                  psi1, psi2);
        r.morphemes1 = uniqArgMorphs1.Morphemes;
        r.morphemes2 = uniqArgMorphs2.Morphemes;
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 7
        // Hack around a GCC 7.x bug where the presence of a move-only data
        // member causes the std::optional constructor to be removed from
        // consideration. This is the only version of GCC we have to worry about
        // (we don't support older versions and the bug was fixed in newer
        // versions).
        return std::move(r);
#else
        return r;
#endif
      }
    }
  }
  return std::nullopt;
}

// Removes low-quality morphemes from the given set. Returns true if removing
// the morphemes leaves the set empty, false otherwise.
static bool removeLowQualityMorphemes(std::set<std::string>& morphemes) {
  // FIXME: implement the reduction heuristics here.
  return morphemes.empty();
}

Checker::Checker(const CheckerConfiguration& opts) : Opts(opts) {
  if (!Opts.ModelPath.empty()) {
    Stats = new Statistics(Opts.ModelPath);
    if (!Stats->valid()) {
      // If we couldn't load valid stats, pretend there were no stats loaded
      // at all rather than leave an invalid database around.
      delete Stats;
      Stats = nullptr;
    }
  }
}

Checker::~Checker() { delete Stats; }

std::vector<Result> Checker::CheckSite(const CallSite& site, Check whichCheck) {
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

    // FIXME: currently, the stub for IdentifierSplitter has no state and
    // requires no parameterization. If that continues to be true after
    // adding the real implementation, this should be replaced with a free
    // function. If it does have state, this may also be more natural as a
    // data member rather than a local.
    IdentifierSplitter splitter;
    MorphemeSet param1Morphemes{splitter.split(param1), pairwiseArgs.first},
        param2Morphemes{splitter.split(param2), pairwiseArgs.second};

    // Do the same thing for arguments, except all argument components are
    // split into the same set. e.g., foo(bar.baz(), 0) may split the first
    // argument into the set [bar, baz]. Verify there is at least one usable
    // morpheme for each argument.
    // FIXME: Currently, the first argument will not produce any morphemes
    // because we've not decided to stick with this approach. If we continue
    // to produce only one identifier per argument, consider flattening the
    // interface of how we represent arguments.
    auto morphemeCollector = [&args, &splitter](MorphemeSet& m, size_t pos) {
      m.Position = pos;
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

    if (!param1.empty() && !param2.empty()) {
      // Having split the parameter identifiers into morphemes, remove any
      // morphemes that are low quality and bail out if there are no usable
      // morphemes left for either parameter. Consider: void foo(int i, int j);
      // as an example of when a morpheme may be of sufficiently low quality to
      // warrant ignoring it.
      if (removeLowQualityMorphemes(param1Morphemes.Morphemes) ||
          removeLowQualityMorphemes(param2Morphemes.Morphemes))
        continue;

      // Run the cover-based checker first.
      if (whichCheck == Check::All || whichCheck == Check::CoverBased) {
        if (std::optional<Result> coverWarning = checkForCoverBasedSwap(
                std::make_pair(param1Morphemes, param2Morphemes),
                std::make_pair(arg1Morphemes, arg2Morphemes), site)) {
          results.push_back(std::move(*coverWarning));
          continue;
        }
      }
    }

    // If that didn't find anything, run the statistics-based checker.
    if ((whichCheck == Check::All || whichCheck == Check::StatsBased) &&
        Stats) {
      assert(Stats->valid() && "Expected valid statistics by this point");

      if (std::optional<Result> statsWarning = checkForStatisticsBasedSwap(
              std::make_pair(param1Morphemes, param2Morphemes),
              std::make_pair(arg1Morphemes, arg2Morphemes), site, *Stats)) {
        results.push_back(std::move(*statsWarning));
      }
    }
  }

  return results;
}
