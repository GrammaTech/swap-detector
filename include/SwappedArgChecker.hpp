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
  float Score;
public:
  explicit UsageStatisticsBasedScoreCard(float score) : Score(score) {}
  CheckerKind kind() const override { return UsageStatisticsBased; }
  float score() const override { return Score; }
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
