#include "SwappedArgChecker.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <variant>
#include <vector>

using namespace swapped_arg;

// Reads a single argument.
static std::string getArg(int idx, int count, char* args[]) {
  if (idx < count)
    return args[idx];
  return "";
}

// Reads a list of arguments until reaching something with a preceding --.
static std::vector<std::string> getArgs(int idx, int count, char* args[]) {
  std::vector<std::string> vals;
  for (int i = idx; i < count; ++i) {
    std::string arg(args[i]);
    if (arg.substr(0, 2) == "--")
      break;
    vals.push_back(arg);
  }
  return vals;
}

std::vector<CallSite::ArgumentNames>
explodeCallSiteArguments(const std::vector<std::string>& args) {
  // The positional argument names is a vector of vector of identifiers so that
  // an argument expression like foo(a + b) would have a single argument
  // comprised of two elements. We currently don't support that for this test
  // interface, so we treat all arguments as being a vector of one.
  std::vector<CallSite::ArgumentNames> ret;
  std::for_each(args.begin(), args.end(),
                [&ret](const std::string& V) { ret.push_back({V}); });
  return ret;
}

// Returns true on success, false on failure. Responsible for printing out the
// error information should the check fail.
static bool runCheck(const std::string& functionName,
                     const std::vector<std::string>& args,
                     const std::optional<std::vector<std::string>>& params,
                     const std::string& callSiteFile, size_t callSiteLineNum,
                     const std::optional<std::string>& callDeclFile,
                     std::optional<size_t> callDeclLineNum) {
  CallDeclDescriptor callDecl;
  callDecl.fullyQualifiedName = functionName;
  callDecl.paramNames = params;

  CallSite site;
  site.callDecl = callDecl;
  site.positionalArgNames = explodeCallSiteArguments(args);

  Checker check;
  std::vector<Result> results = check.CheckSite(site);
  for (const auto& res : results) {
    std::cerr << "ERROR (" << callSiteFile << ":" << callSiteLineNum
              << "): " << site.callDecl.fullyQualifiedName
              << " has swapped arguments " << res.arg1 << " and " << res.arg2
              << " with a score of " << res.score->score() << std::endl;
    std::cerr << "NOTE (" << callDeclFile.value_or("<unknown>") << ":"
              << callDeclLineNum.value_or(std::numeric_limits<size_t>::max())
              << "): callee declared here" << std::endl;
  }
  return results.empty();
}

static void printUsage() {
  std::cout << "--function <name>\n"
            << "--call_site_file <name>\n"
            << "--call_site_line <number>\n"
            << "--args <arg1, arg2, ...argN>\n"
            << "*OPTIONAL* --call_decl_file <name>\n"
            << "*OPTIONAL* --call_decl_line <number>\n"
            << "*OPTIONAL* --params <param1, param2, ...paramN>\n";
}

int main(int argc, char* argv[]) {
  std::string functionName, callSiteFileName;
  size_t callSiteLineNumber = std::numeric_limits<size_t>::max();
  std::optional<std::string> callDeclFileName;
  std::optional<size_t> callDeclLineNumber;
  std::vector<std::string> args;
  std::optional<std::vector<std::string>> params;

  for (int i = 0; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "--function") {
      functionName = getArg(i + 1, argc, argv);
      i += 1;
    } else if (arg == "--args") {
      args = getArgs(i + 1, argc, argv);
      i += static_cast<int>(args.size());
    } else if (arg == "--params") {
      params = getArgs(i + 1, argc, argv);
      i += static_cast<int>(args.size());
    } else if (arg == "--call_site_file") {
      callSiteFileName = getArg(i + 1, argc, argv);
      i += 1;
    } else if (arg == "--call_site_line") {
      std::string line = getArg(i + 1, argc, argv);
      if (!line.empty())
        callSiteLineNumber = static_cast<size_t>(std::stoull(line));
      i += 1;
    } else if (arg == "--call_decl_file") {
      callDeclFileName = getArg(i + 1, argc, argv);
      i += 1;
    } else if (arg == "--call_decl_line") {
      std::string line = getArg(i + 1, argc, argv);
      if (!line.empty())
        callDeclLineNumber = static_cast<size_t>(std::stoull(line));
      i += 1;
    }
  }

  // Sanity checking.
  if (functionName.empty() || callSiteFileName.empty()) {
    printUsage();
    return EXIT_FAILURE;
  }

  if (!runCheck(functionName, args, params, callSiteFileName,
                callSiteLineNumber, callDeclFileName, callDeclLineNumber))
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
