#ifndef GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H
#define GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H

#include "Compiler.hpp"
#include <string>
#include <vector>

namespace swapped_arg {
class SWAPPED_ARG_EXPORT IdentifierSplitter {
public:
  std::vector<std::string> split(const std::string& input) const;
};
} // end namespace swapped_arg

#endif // GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H
