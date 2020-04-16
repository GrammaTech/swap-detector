#include "gtest/gtest.h"
#include "SwappedArgChecker.hpp"

using namespace swapped_arg;

TEST(Swapping, Stuff) {
  EXPECT_TRUE(true);

  CheckerConfiguration Opts{"test.foo"};
  Checker C(Opts);
}
