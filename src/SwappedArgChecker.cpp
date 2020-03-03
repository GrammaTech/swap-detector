#include "SwappedArgChecker.hpp"
#include <random>

std::string Result::debug_str() const { return ""; }

Checker::Checker(std::string model_path) {}
void Checker::CheckSite(const CallSite& site,
                        std::function<void(const Result&)> report_callback) {
  std::random_device rd;
  std::mt19937 gen(rd());
  if (std::generate_canonical<double, 32>(gen) < .25) {
    Result R;
    // Pick two random arguments to consider swapped.
    R.arg1 = static_cast<size_t>(std::generate_canonical<double, 32>(gen) *
                                 site.positional_arg_names.size() + 1);
    size_t Pos = std::get<size_t>(R.arg1);
    // The swapped argument will either be the one right after the random
    // choice, or right before (if the random choice was the last argument).
    R.arg2 = Pos != site.positional_arg_names.size() ? Pos + 1 : Pos - 1;
    R.score = std::generate_canonical<double, 32>(gen) * 100.0;
    report_callback(R);
  }
}
