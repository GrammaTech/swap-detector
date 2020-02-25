#include "SwappedArgChecker.hpp"

std::string Result::debug_str() const {
  return "";
}

Checker::Checker(std::string model_path) {}
void Checker::CheckSite(const CallSite &site,
                        std::function<void(const Result &)> report_callback) {}
