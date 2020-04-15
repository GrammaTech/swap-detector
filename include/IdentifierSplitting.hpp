#ifndef GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H
#define GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H

#include "Compiler.hpp"
#include <set>
#include <string>

namespace swapped_arg {
class SWAPPED_ARG_EXPORT IdentifierSplitter {
public:
  // Returns a set of unique morphemes for the identifier. e.g.,
  // foo_barBaz_bar would result in a set [foo, bar, Baz], not
  // [foo, bar, Baz, bar].
  std::set<std::string> split(const std::string& input) const;
};
} // end namespace swapped_arg

#endif // GT_SWAPPED_ARG_IDENTIFIER_SPLITTING_H
