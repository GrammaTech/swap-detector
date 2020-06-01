#ifndef GT_SWAPPED_ARG_CHECKER_H
#define GT_SWAPPED_ARG_CHECKER_H

#include "Compiler.hpp"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace swapped_arg {

// A description of a function being called.
class CallDeclDescriptor {
public:
  // The fully qualified name of the callee.
  std::string fullyQualifiedName;

  // The names of the formal parameters from the callee.
  std::optional<std::vector<std::string>> paramNames;

  std::optional<bool> isVariadicFunction;
};

// A single call-site to check for swapped argument errors.
class CallSite {
public:
  using ArgumentNames = std::vector<std::string>;

  // Details about the callee of the call site.
  CallDeclDescriptor callDecl;

  // Name expressions for each positional argument.
  std::vector<ArgumentNames> positionalArgNames;
};

// Basic functionality to represent a score card from a failing check result.
class ScoreCard {
public:
  // What kind of checking strategies are supported.
  enum CheckerKind {
    ParameterNameBased,
    UsageStatisticsBased,
  };

  virtual ~ScoreCard() = default;

  // The checker's confidence in this being a true positive, 0-100. Tools
  // can map this value to be in their "native" range.
  virtual float score() const = 0;

  // The kind of checker the score card provides results for.
  virtual CheckerKind kind() const = 0;
};

class ParameterNameBasedScoreCard : public ScoreCard {
  float Score;
  bool WasStatsCheckerRun;

public:
  explicit ParameterNameBasedScoreCard(float score, bool statsChecked)
      : Score(score), WasStatsCheckerRun(statsChecked) {}
  CheckerKind kind() const override { return ParameterNameBased; }
  float score() const override { return Score; }
  bool wasStatsCheckerRun() const { return WasStatsCheckerRun; }
};

class UsageStatisticsBasedScoreCard : public ScoreCard {
  float Fit1, Fit2;

public:
  explicit UsageStatisticsBasedScoreCard(float fit1, float fit2)
      : Fit1(fit1), Fit2(fit2) {}
  CheckerKind kind() const override { return UsageStatisticsBased; }
  float score() const override { return std::max(Fit1, Fit2); }
  float arg1_fitness() const { return Fit1; }
  float arg2_fitness() const { return Fit2; }
};

// A swapped argument error.
class Result {
public:
  // Indices of the swapped arguments. Integer argument indexes are one-based.
  size_t arg1;
  size_t arg2;

  // The specific morphemes in each argument that were swapped.
  std::set<std::string> morphemes1, morphemes2;

  std::unique_ptr<ScoreCard> score;

  std::string debugStr() const;
};

struct SWAPPED_ARG_EXPORT CheckerConfiguration {
  // Filesystem-native path to the model database.
  std::string ModelPath;
  // Comparison values used after calculating the match liklihood for either
  // pessimistic or optimistic matching, respectively.
  float ExistingMorphemeMatchMax = 0.5f;
  float SwappedMorphemeMatchMin = 0.75f;
  // Comparison value used to determine whether an argument morpheme is
  // statistically likely to be a swap.
  float StatsSwappedMorphemeThreshold = 0.75f; // FIXME: made up number!!
  // Comparison value used to determine whether a potential swap is
  // sufficiently fit or not.
  float StatsSwappedFitnessThreshold = 0.75f; // FIXME: made up number!!
};

class Statistics {
  // FIXME: This maps the function, position, morpheme tuple to a score. It
  // will be replaced by a real database someday, I hope.
  std::map<std::string, std::map<size_t, std::map<std::string, float>>> MorphDB;

public:
  // This function will eventually be deleted, but exists so that we can get
  // some use out of the interface. It should be replaced by a constructor that
  // loads the statistics database instead.
  void setWeightForMorpheme(const std::string& funcName, size_t argPos,
                            const std::string& morpheme, float value) {
    MorphDB[funcName][argPos][morpheme] = value;
  }
  // Determine the relative frequency of the given morpheme compared to any
  // other morpheme in given position. Returns a value between [0, 1) such that
  // the sum of weights for all morphemes that occur in that position is 1.
  float weightForMorpheme(const std::string& funcName, size_t argPos,
                          const std::string& morpheme) const {
    if (auto funcIt = MorphDB.find(funcName); funcIt != MorphDB.end()) {
      if (auto posIt = funcIt->second.find(argPos);
          posIt != funcIt->second.end()) {
        if (auto morphIt = posIt->second.find(morpheme);
            morphIt != posIt->second.end()) {
          return morphIt->second;
        }
      }
    }
    return 0.0f;
  }

  bool morphemesAtPos(const std::string& funcName, size_t argPos,
                      std::vector<std::string>& morphemes) const {
    if (auto funcIt = MorphDB.find(funcName); funcIt != MorphDB.end()) {
      if (auto posIt = funcIt->second.find(argPos);
          posIt != funcIt->second.end()) {
        for (const auto& kvpair : posIt->second) {
          morphemes.push_back(kvpair.first);
        }
        return true;
      }
    }
    return false;
  }
};

class SWAPPED_ARG_EXPORT Checker {
  CheckerConfiguration Opts;

  // Get the parameter name, if any, at the given zero-based index.
  std::optional<std::string> getParamName(const CallSite& site,
                                          size_t pos) const {
    if (!site.callDecl.paramNames)
      return std::nullopt;
    if (pos >= site.callDecl.paramNames->size())
      return std::nullopt;
    return (*site.callDecl.paramNames)[pos];
  }

  struct MorphemeSet {
    std::set<std::string> Morphemes;
    size_t Position;
  };

  MorphemeSet morphemeSetDifference(const MorphemeSet& one,
                                    const MorphemeSet& two) const;

  // Gets the last identifier in the argument name, if any, at the given
  // zero-based index.
  std::optional<std::string> getLastArgName(const CallSite& site,
                                            size_t pos) const {
    if (pos >= site.positionalArgNames.size())
      return std::nullopt;
    return site.positionalArgNames[pos].back();
  }

  // TODO: remove this debugging utility when done.
  void print(const MorphemeSet& m, bool isArg);

  std::optional<Result>
  checkForCoverBasedSwap(const std::pair<MorphemeSet, MorphemeSet>& params,
                         const std::pair<MorphemeSet, MorphemeSet>& args,
                         const CallSite& callSite);

  float anyAreSynonyms(const std::string& morpheme,
                       const std::set<std::string>& potentialSynonyms) const;

  // Helper struct for comparing against the bias when matching morphemes.
  enum class Bias { Pessimistic, Optimistic };
  struct BiasComp {
    explicit BiasComp(Bias b, const CheckerConfiguration& Opts)
        : Less(b == Bias::Pessimistic) {}

    bool operator()(float lhs, float rhs) const {
      if (Less)
        return lhs < rhs;
      return lhs > rhs;
    }
  private:
    bool Less;
  };

  std::set<std::string>
  nonLowEntropyDifference(const std::set<std::string>& lhs,
                          const std::set<std::string>& rhs) const;

  float morphemesMatch(const std::set<std::string>& arg,
                       const std::set<std::string>& param, Bias bias) const;

  std::optional<Result>
  checkForStatisticsBasedSwap(const std::pair<MorphemeSet, MorphemeSet>& params,
                              const std::pair<MorphemeSet, MorphemeSet>& args,
                              const CallSite& callSite,
                              const Statistics& stats);
  // Determines the confidence of how much more common it is to see the given
  // morpheme at the given position compared to another position. Returns values
  // in the range 0.0f (for no confidence) to infinity (for highest confidence).
  // FIXME: remove the paramMorphs parameter when doing the real implementation.
  float
  morphemeConfidenceAtPosition(const std::string& morph, size_t pos,
                               size_t comparedToPos,
                               const std::set<std::string>& paramMoprphs) const;

  // Determines how "similar" two morphemes are, including abbreviations and
  // synonyms. Returns a value between [0, 1).
  float similarity(const std::string& morph1, const std::string& morph2) const;

  // Determines the fitness of a potential swap of the given morpheme when
  // compared to the other morphemes used at that position in other function
  // calls. Returns a value between [0, 1).
  float fit(const std::string& morph, const CallSite& site, size_t argPos,
            const Statistics& stats) const;

public:
  Checker() = default;
  explicit Checker(const CheckerConfiguration& opts) : Opts(opts) {}

  // Checks for all argument swap errors at a given call site.
  // @param site Details about the call site.
  // @return All of the dected swaps at the site.
  std::vector<Result> CheckSite(const CallSite& site);

  const CheckerConfiguration& Options() const { return Opts; }
  void setOptions(const CheckerConfiguration& opts) {
    Opts = opts;
  }
};

} // end namespace swapped_arg
#endif // GT_SWAPPED_ARG_CHECKER_H
