#include "IdentifierSplitting.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace swapped_arg;

TEST(IdentifierSplitting, split) {
  IdentifierSplitter Splitter;

  EXPECT_THAT(Splitter.split("foo_bar"),
              testing::UnorderedElementsAre("foo", "bar"));
  EXPECT_THAT(Splitter.split("fooBar"),
              testing::UnorderedElementsAre("foo", "bar"));
  EXPECT_THAT(Splitter.split("Foobar"),
              testing::UnorderedElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("fooBarBaz"),
              testing::UnorderedElementsAre("foo", "bar", "baz"));
  EXPECT_THAT(Splitter.split("foo_barBaz"),
              testing::UnorderedElementsAre("foo", "bar", "baz"));

  EXPECT_THAT(Splitter.split("__foobar"),
              testing::UnorderedElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("foobar__"),
              testing::UnorderedElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("foo__bar"),
              testing::UnorderedElementsAre("foo", "bar"));

  EXPECT_THAT(Splitter.split("FOOBAR"),
              testing::UnorderedElementsAre("foobar"));
  EXPECT_THAT(Splitter.split("fooBAR"),
              testing::UnorderedElementsAre("foo", "bar"));
  EXPECT_THAT(Splitter.split("FOObar"),
              testing::UnorderedElementsAre("foobar"));

  EXPECT_THAT(Splitter.split("foo_bar_bar"),
              testing::UnorderedElementsAre("foo", "bar"));
}
