#include "SwappedArgChecker.hpp"
#include "gmock/gmock.h"
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
  EXPECT_THAT(ActualResult.morphemes1, testing::UnorderedElementsAre("dogs"));
  EXPECT_THAT(ActualResult.morphemes2, testing::UnorderedElementsAre("cats"));
}

TEST(Swapping, DifferentMorphemeCases) {
  // Ensure that case does not matter when finding a swap.
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "DifferentMorphemeCasesTest";
  Site.callDecl.paramNames = {"Dogs", "Cats"};
  Site.positionalArgNames = {{"cats"}, {"dogs"}};

  Result ActualResult;
  C.CheckSite(Site, [&ActualResult](const Result& R) {
    ActualResult = R;
    EXPECT_NE(R.score, nullptr);
    ActualResult.score = nullptr; // Avoid double delete
  });

  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 1);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 2);
  EXPECT_THAT(ActualResult.morphemes1, testing::UnorderedElementsAre("cats"));
  EXPECT_THAT(ActualResult.morphemes2, testing::UnorderedElementsAre("dogs"));
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

  unsigned NumResults = 0;
  C.CheckSite(Site, [&NumResults](const Result& R) { ++NumResults; });
  EXPECT_EQ(NumResults, 0);
}

TEST(Swapping, ShouldNotMatch) {
  Checker C;

  // Test that we don't warn when argument and parameter names match exactly.
  CallSite Site1;
  Site1.callDecl.fullyQualifiedName = "NegativeTest";
  Site1.callDecl.paramNames = {"horses", "emus"};
  Site1.positionalArgNames = {{"horses"}, {"emus"}};

  unsigned NumResults = 0;
  C.CheckSite(Site1, [&NumResults](const Result& R) { ++NumResults; });
  EXPECT_EQ(NumResults, 0);

  // Test that we don't warn when argument and parameter names don't relate.
  CallSite Site2;
  Site2.callDecl.fullyQualifiedName = "NegativeTest";
  Site2.callDecl.paramNames = {"horses", "emus"};
  Site2.positionalArgNames = {{"ponies"}, {"ostriches"}};

  NumResults = 0;
  C.CheckSite(Site2, [&NumResults](const Result& R) { ++NumResults; });
  EXPECT_EQ(NumResults, 0);

  // Test that we don't warn when the argument morphemes do not fully cover
  // the parameter morphemes.
  CallSite Site3;
  Site3.callDecl.fullyQualifiedName = "UncoveredMorpheme";
  Site3.callDecl.paramNames = {"barking_dogs", "hissing_cats"};
  Site3.positionalArgNames = {{"silly_cats"}, {"dogs_lolling"}};

  NumResults = 0;
  C.CheckSite(Site3, [&NumResults](const Result& R) { ++NumResults; });
  EXPECT_EQ(NumResults, 0);

  // Morphemes are not fully covered and it's a rotation rather than a swap.
  CallSite Site4;
  Site4.callDecl.fullyQualifiedName = "UncoverdMorphemeRotation";
  Site4.callDecl.paramNames = {"barking_dogs", "hissing_cats",
                               "running_alligators", "flailing_nudibranches"};
  Site4.positionalArgNames = {{"barfing_nudibranches"},
                              {"dogs_lolling"},
                              {"purring_cats"},
                              {"alligators_eating"}};

  NumResults = 0;
  C.CheckSite(Site4, [&NumResults](const Result& R) { ++NumResults; });
  EXPECT_EQ(NumResults, 0);
}

TEST(Swapping, MultipleMorphemes) {
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "MultipleMorphemesTest";
  Site.callDecl.paramNames = {"lolling_dogs", "cats_silly"};
  Site.positionalArgNames = {{"silly_cats"}, {"dogs_lolling"}};

  Result ActualResult;
  C.CheckSite(Site, [&ActualResult](const Result& R) {
    ActualResult = R;
    EXPECT_NE(R.score, nullptr);
    ActualResult.score = nullptr; // Avoid double delete
  });

  EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 1);
  EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 2);
  EXPECT_THAT(ActualResult.morphemes1,
              testing::UnorderedElementsAre("cats", "silly"));
  EXPECT_THAT(ActualResult.morphemes2,
              testing::UnorderedElementsAre("dogs", "lolling"));
}

TEST(Swapping, NumericSuffixes) {
  Checker C;

  // Test that we don't match when the param suffixes are the same, even if
  // there would be a swap otherwise.
  CallSite Site1;
  Site1.callDecl.fullyQualifiedName = "NegativeNumericSuffixTestParam";
  Site1.callDecl.paramNames = {"horses1", "horses2"};
  Site1.positionalArgNames = {{"horses2"}, {"horses1"}};

  unsigned NumResults = 0;
  C.CheckSite(Site1, [&NumResults](const Result& R) { ++NumResults; });
  EXPECT_EQ(NumResults, 0);

  // Similarly, test that we don't match when the arg suffixes are the same,
  // even if there would be a swap otherwise.
  CallSite Site2;
  Site2.callDecl.fullyQualifiedName = "NegativeNumericSuffixTestArg";
  Site2.callDecl.paramNames = {"horses", "horses"}; // Fake data is fake.
  Site2.positionalArgNames = {{"horses1", "horses2"}};

  NumResults = 0;
  C.CheckSite(Site2, [&NumResults](const Result& R) { ++NumResults; });
  EXPECT_EQ(NumResults, 0);

  // However, we should still catch swaps like this one.
  // FIXME: this test case fails because we do not handle abbreviations when
  // checking whether morphemes match and so we do not consider goats1 to be
  // a swap with goats.
  CallSite Site3;
  Site3.callDecl.fullyQualifiedName = "PositiveNumericSuffixTest";
  Site3.callDecl.paramNames = {"horses", "goats"};
  Site3.positionalArgNames = {{"goats1"}, {"horses2"}};

  Result ActualResult;
  C.CheckSite(Site3, [&ActualResult](const Result& R) {
    ActualResult = R;
    EXPECT_NE(R.score, nullptr);
    ActualResult.score = nullptr; // Avoid double delete
  });
  /*
    EXPECT_EQ(std::get<size_t>(ActualResult.arg1), 1);
    EXPECT_EQ(std::get<size_t>(ActualResult.arg2), 2);
    EXPECT_THAT(ActualResult.morphemes1,
    testing::UnorderedElementsAre("goats1"));
    EXPECT_THAT(ActualResult.morphemes2,
    testing::UnorderedElementsAre("horses2"));
  */
}
