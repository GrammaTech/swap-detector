#include "SwappedArgChecker.hpp"
#include <random>

using namespace swapped_arg;

std::string Result::debugStr() const { return ""; }

Checker::Checker(std::string modelPath) {}
void Checker::CheckSite(const CallSite& site,
                        std::function<void(const Result&)> reportCallback) {
  std::random_device rd;
  std::mt19937 gen(rd());
  if (std::generate_canonical<double, 32>(gen) < .25) {
    Result r;
    // Pick two random arguments to consider swapped.
    r.arg1 = static_cast<size_t>(std::generate_canonical<double, 32>(gen) *
                                 site.positionalArgNames.size() + 1);
    size_t pos = std::get<size_t>(r.arg1);
    // The swapped argument will either be the one right after the random
    // choice, or right before (if the random choice was the last argument).
    r.arg2 = pos != site.positionalArgNames.size() ? pos + 1 : pos - 1;
    r.score = std::generate_canonical<double, 32>(gen) * 100.0;
    reportCallback(r);
  }
}
