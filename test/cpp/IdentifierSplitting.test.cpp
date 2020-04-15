#include "IdentifierSplitting.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace swapped_arg;

TEST(IdentifierSplitting, split) {
  IdentifierSplitter Splitter;

  EXPECT_THAT(Splitter.split("foo_bar"),
              testing::UnorderedElementsAre("foo", "bar"));
  EXPECT_THAT(Splitter.split("fooBar"),
              testing::UnorderedElementsAre("foo", "Bar"));
  EXPECT_THAT(Splitter.split("Foobar"),
              testing::UnorderedElementsAre("Foobar"));
  EXPECT_THAT(Splitter.split("fooBarBaz"),
              testing::UnorderedElementsAre("foo", "Bar", "Baz"));
  EXPECT_THAT(Splitter.split("foo_barBaz"),
              testing::UnorderedElementsAre("foo", "bar", "Baz"));

  EXPECT_THAT(Splitter.split("__foobar"),
              testing::UnorderedElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("foobar__"),
              testing::UnorderedElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("foo__bar"),
              testing::UnorderedElementsAre("foo", "bar"));

  EXPECT_THAT(Splitter.split("FOOBAR"),
              testing::UnorderedElementsAre("FOOBAR"));
  EXPECT_THAT(Splitter.split("fooBAR"),
              testing::UnorderedElementsAre("foo", "BAR"));
  EXPECT_THAT(Splitter.split("FOObar"),
              testing::UnorderedElementsAre("FOObar"));

  EXPECT_THAT(Splitter.split("foo_bar_bar"),
              testing::UnorderedElementsAre("foo", "bar"));
}
