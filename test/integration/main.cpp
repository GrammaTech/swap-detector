#include "SwappedArgChecker.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <optional>
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

// Returns true on success, false on failure. Responsible for printing out the
// error information should the check fail.
static bool runCheck(const std::string& FunctionName,
                     const std::vector<std::string>& Args,
                     const std::optional<std::vector<std::string>>& Params,
                     const std::string& CallerFile, size_t CallerLineNum,
                     const std::optional<std::string>& CalleeFile,
                     std::optional<size_t> CalleeLineNum) {
  CalleeDescriptor Callee;
  Callee.callee = FunctionName;
  Callee.param_names = Params;

  CallSite Site;
  Site.callee = Callee;
  Site.positional_arg_names = explodeCallSiteArguments(Args);

  bool Failed = false;
  Checker Check;
  Check.CheckSite(Site, [&](const Result& R) {
    Failed = true;
    std::cerr << "ERROR (" << CallerFile << ":" << CallerLineNum
              << "): " << Site.callee.callee << " has swapped arguments "
              << std::get<size_t>(R.arg1) << " and " << std::get<size_t>(R.arg2)
              << " with a score of " << R.score << std::endl;
    std::cerr << "NOTE (" << CalleeFile.value_or("<unknown>") << ":"
              << CalleeLineNum.value_or(std::numeric_limits<size_t>::max())
              << "): callee declared here" << std::endl;
  });
  return !Failed;
}

static void printUsage() {
  std::cout << "--function <name>\n"
            << "--caller_file <name>\n"
            << "--caller_line <number>\n"
            << "--args <arg1, arg2, ...argN>\n"
            << "*OPTIONAL* --callee_file <name>\n"
            << "*OPTIONAL* --callee_line <number>\n"
            << "*OPTIONAL* --params <param1, param2, ...paramN>\n";
}

int main(int argc, char* argv[]) {
  std::string FunctionName, CallerFileName;
  size_t CallerLineNumber = std::numeric_limits<size_t>::max();
  std::optional<std::string> CalleeFileName;
  std::optional<size_t> CalleeLineNumber;
  std::vector<std::string> Args;
  std::optional<std::vector<std::string>> Params;

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
    } else if (Arg == "--caller_file") {
      CallerFileName = getArg(i + 1, argc, argv);
      i += 1;
    } else if (Arg == "--caller_line") {
      std::string Arg = getArg(i + 1, argc, argv);
      if (!Arg.empty())
        CallerLineNumber = static_cast<size_t>(std::stoull(Arg));
      i += 1;
    } else if (Arg == "--callee_file") {
      CalleeFileName = getArg(i + 1, argc, argv);
      i += 1;
    } else if (Arg == "--callee_line") {
      std::string Arg = getArg(i + 1, argc, argv);
      if (!Arg.empty())
        CalleeLineNumber = static_cast<size_t>(std::stoull(Arg));
      i += 1;
    }
  }

  // Sanity checking.
  if (FunctionName.empty() || CallerFileName.empty()) {
    printUsage();
    return EXIT_FAILURE;
  }

  if (!runCheck(FunctionName, Args, Params, CallerFileName, CallerLineNumber,
                CalleeFileName, CalleeLineNumber))
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
