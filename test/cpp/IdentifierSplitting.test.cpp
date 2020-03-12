#include "IdentifierSplitting.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace swapped_arg;

TEST(IdentifierSplitting, split) {
  IdentifierSplitter Splitter;

  EXPECT_THAT(Splitter.split("foo_bar"), testing::ElementsAre("foo", "bar"));
  EXPECT_THAT(Splitter.split("fooBar"), testing::ElementsAre("foo", "Bar"));
  EXPECT_THAT(Splitter.split("Foobar"), testing::ElementsAre("Foobar"));
  EXPECT_THAT(Splitter.split("fooBarBaz"),
              testing::ElementsAre("foo", "Bar", "Baz"));
  EXPECT_THAT(Splitter.split("foo_barBaz"),
              testing::ElementsAre("foo", "bar", "Baz"));

  EXPECT_THAT(Splitter.split("__foobar"), testing::ElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("foobar__"), testing::ElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("foo__bar"), testing::ElementsAre("foo", "bar"));

  EXPECT_THAT(Splitter.split("FOOBAR"), testing::ElementsAre("FOOBAR"));
  EXPECT_THAT(Splitter.split("fooBAR"), testing::ElementsAre("foo", "BAR"));
  EXPECT_THAT(Splitter.split("FOObar"), testing::ElementsAre("FOObar"));
}
