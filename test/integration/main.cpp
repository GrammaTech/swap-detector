#include "SwappedArgChecker.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

// Reads a single argument.
static std::string getArg(int Idx, int Count, char* Args[]) {
  if (Idx < Count)
    return Args[Idx];
  return "";
}

// Reads a list of arguments until reaching something with a preceding --.
static std::vector<std::string> getArgs(int Idx, int Count, char* Args[]) {
  std::vector<std::string> Vals;
  for (int i = Idx; i < Count; ++i) {
    std::string Arg(Args[i]);
    if (Arg.substr(0, 2) == "--")
      break;
    Vals.push_back(Arg);
  }
  return Vals;
}

std::vector<CallSite::ArgumentNames>
explodeCallSiteArguments(const std::vector<std::string>& Args) {
  // The positional argument names is a vector of vector of identifiers so that
  // an argument expression like foo(a + b) would have a single argument
  // comprised of two elements. We currently don't support that for this test
  // interface, so we treat all arguments as being a vector of one.
  std::vector<CallSite::ArgumentNames> Ret;
  std::for_each(Args.begin(), Args.end(),
                [&Ret](const std::string& V) { Ret.push_back({V}); });
  return Ret;
}

static bool RunCheck(const std::string& FunctionName,
                     const std::vector<std::string>& Args,
                     const std::vector<std::string>& Params) {
  CalleeDescriptor Callee;
  Callee.callee = FunctionName;
  Callee.param_names = Params;

  CallSite Site;
  Site.callee = Callee;
  Site.positional_arg_names = explodeCallSiteArguments(Args);

  bool Failed = false;
  Checker Check;
  Check.CheckSite(Site, [&Failed, &Site](const Result& R) {
    Failed = true;
    std::cerr << "ERROR: " << Site.callee.callee << " has swapped arguments "
              << std::get<size_t>(R.arg1) << " and " << std::get<size_t>(R.arg2)
              << " with a score of " << R.score << std::endl;
    });
  return Failed;
}

// Usage: <app> --function <name>
//              --args <arg1, ...argN>
//              --params <param1, ...paramN>
int main(int argc, char* argv[]) {
  std::string FunctionName;
  std::vector<std::string> Args;
  std::vector<std::string> Params;

  for (int i = 0; i < argc; ++i) {
    std::string Arg(argv[i]);
    if (Arg == "--function") {
      FunctionName = getArg(i + 1, argc, argv);
      i += 1;
    } else if (Arg == "--args") {
      Args = getArgs(i + 1, argc, argv);
      i += static_cast<int>(Args.size());
    } else if (Arg == "--params") {
      Params = getArgs(i + 1, argc, argv);
      i += static_cast<int>(Args.size());
    }
  }

  // Sanity checking.
  if (FunctionName.empty() || Args.size() != Params.size())
    return EXIT_FAILURE;

  RunCheck(FunctionName, Args, Params);
}
