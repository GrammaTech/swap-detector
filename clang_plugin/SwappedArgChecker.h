#ifndef PLUGIN_SWAPPEDARGCHECKER_H
#define PLUGIN_SWAPPEDARGCHECKER_H

#include <clang/StaticAnalyzer/Core/BugReporter/BugReporter.h>
#include <clang/StaticAnalyzer/Core/BugReporter/BugType.h>
#include <clang/StaticAnalyzer/Core/Checker.h>
#include "SwappedArgChecker.hpp"

using namespace clang;
using namespace clang::ento;

class SwappedArgChecker : public Checker<check::PreCall> {
  mutable swapped_arg::Checker Check;
  mutable std::unique_ptr<BugType> BT;

  friend class check::PreCall;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void reportRuleViolation(const CallEvent &Call, CheckerContext &C, size_t Arg1,
                           size_t Arg2, llvm::StringRef Message) const;

public:
  SwappedArgChecker(const std::string &modelPath);
};

#endif // PLUGIN_SWAPPEDARGCHECKER_H

