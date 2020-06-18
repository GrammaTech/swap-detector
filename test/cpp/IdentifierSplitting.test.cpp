//===- IdentifierSplitting.test.cpp -----------------------------*- C++ -*-===//
//
//  Copyright (C) 2020 GrammaTech, Inc.
//
//  This code is licensed under the MIT license. See the LICENSE file in the
//  project root for license terms.
//
// This material is based on research sponsored by the Department of Homeland
// Security (DHS) Office of Procurement Operations, S&T acquisition Division via
// contract number 70RSAT19C00000056. The views and conclusions contained herein
// are those of the authors and should not be interpreted as necessarily
// representing the official policies or endorsements, either expressed or
// implied, of the Department of Homeland Security.
//
//===----------------------------------------------------------------------===//
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
