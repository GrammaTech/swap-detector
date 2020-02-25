#ifndef GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H
#define GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H

#include "Compiler.hpp"
#include <string>
#include <vector>

class SWAPPED_ARG_EXPORT IdentifierSplitter {
public:
  std::vector<std::string> split(const std::string& Input) const;
};

#endif // GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H

