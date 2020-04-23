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
    ActualResult.score = nullptr; // Avoid double delete
  });

  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 1);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 2);
  EXPECT_EQ(ActualResult.morpheme1, "cats");
  EXPECT_EQ(ActualResult.morpheme2, "dogs");
}

TEST(Swapping, DifferentMorphemeCases) {
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "DifferentMorphemeCasesTest";
  Site.callDecl.paramNames = {"barkingDogs", "hissingCats"};
  Site.positionalArgNames = {{"catsRule"}, {"dogsDrool"}};

  Result ActualResult1;
  C.CheckSite(Site, [&ActualResult1](const Result& R) {
    ActualResult1 = R;
    EXPECT_NE(R.score, nullptr);
    ActualResult1.score = nullptr; // Avoid double delete;
  });

  EXPECT_EQ(std::get<size_t>(ActualResult1.arg1), 1);
  EXPECT_EQ(std::get<size_t>(ActualResult1.arg2), 2);
  EXPECT_EQ(ActualResult1.morpheme1, "cats");
  EXPECT_EQ(ActualResult1.morpheme2, "dogs");

  Site.callDecl.fullyQualifiedName = "DifferentMorphemeCasesTest";
  Site.callDecl.paramNames = {"barking_dogs", "hissing_cats"};
  Site.positionalArgNames = {{"CatsRule"}, {"DogsDrool"}};

  Result ActualResult2;
  C.CheckSite(Site, [&ActualResult2](const Result& R) {
    ActualResult2 = R;
    EXPECT_NE(R.score, nullptr);
    ActualResult2.score = nullptr; // Avoid double delete;
  });

  EXPECT_EQ(std::get<size_t>(ActualResult2.arg1), 1);
  EXPECT_EQ(std::get<size_t>(ActualResult2.arg2), 2);
  EXPECT_EQ(ActualResult2.morpheme1, "cats");
  EXPECT_EQ(ActualResult2.morpheme2, "dogs");
}

TEST(Swapping, DifferentMorphemeCounts) {
  // Currently we do not expect to find cover-based swaps when the number of
  // morphemes in the arguments does not match the number of morphemes in the
  // parameters.
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "DifferentMorphemeCountsTest";
  Site.callDecl.paramNames = {"barking_dogs", "hissing_cats"};
  Site.positionalArgNames = {{"cats"}, {"dogs"}};

  Result ActualResult;
  C.CheckSite(Site, [&ActualResult](const Result& R) { ActualResult = R; });

  EXPECT_EQ(ActualResult.score, nullptr);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 0);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 0);
  EXPECT_EQ(ActualResult.morpheme1, "");
  EXPECT_EQ(ActualResult.morpheme2, "");
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
