#include "SwappedArgChecker.hpp"
#include "gtest/gtest.h"

using namespace swapped_arg;

TEST(Swapping, Basics) {
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "BasicTest";
  Site.callDecl.paramNames = {"cats", "dogs"};
  Site.positionalArgNames = {{"dogs"}, {"cats"}};

  Result ActualResult;
  C.CheckSite(Site, [&ActualResult](const Result& R) {
    ActualResult = R;
    EXPECT_NE(R.score, nullptr);
    ActualResult.score = nullptr; // Avoid double delete
  });

  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 1);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 2);
  EXPECT_EQ(ActualResult.morpheme1, "dogs");
  EXPECT_EQ(ActualResult.morpheme2, "cats");
}

TEST(Swapping, MultipleMorphemes) {
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "MultipleMorphsTest";
  Site.callDecl.paramNames = {"stupid_dogs_that_drool",
                              "which_cats_are_hissing"};
  Site.positionalArgNames = {{"do_chartreuse_cats_exist"},
                             {"why_do_dogs_blink"}};

  Result ActualResult;
  C.CheckSite(Site, [&ActualResult](const Result& R) {
    ActualResult = R;
    EXPECT_NE(R.score, nullptr);
    ActualResult.score = nullptr; // Avoid double  delete
  });

  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 1);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 2);
  EXPECT_EQ(ActualResult.morpheme1, "cats");
  EXPECT_EQ(ActualResult.morpheme2, "dogs");
}

TEST(Swapping, ShouldNotMatch) {
  Checker C;

  // Test that we don't warn when argument and parameter names match exactly.
  CallSite Site1;
  Site1.callDecl.fullyQualifiedName = "NegativeTest";
  Site1.callDecl.paramNames = {"horses", "emus"};
  Site1.positionalArgNames = {{"horses"}, {"emus"}};

  Result ActualResult;
  C.CheckSite(Site1, [&ActualResult](const Result& R) { ActualResult = R; });

  EXPECT_EQ(ActualResult.score, nullptr);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 0);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 0);
  EXPECT_EQ(ActualResult.morpheme1, "");
  EXPECT_EQ(ActualResult.morpheme2, "");

  // Test that we don't warn when argument and parameter names don't relate.
  CallSite Site2;
  Site2.callDecl.fullyQualifiedName = "NegativeTest";
  Site2.callDecl.paramNames = {"horses", "emus"};
  Site2.positionalArgNames = {{"ponies"}, {"ostriches"}};

  C.CheckSite(Site2, [&ActualResult](const Result& R) { ActualResult = R; });

  EXPECT_EQ(ActualResult.score, nullptr);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 0);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 0);
  EXPECT_EQ(ActualResult.morpheme1, "");
  EXPECT_EQ(ActualResult.morpheme2, "");
}
