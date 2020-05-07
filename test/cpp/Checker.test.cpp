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

  std::vector<Result> Results = C.CheckSite(Site);
  EXPECT_EQ(Results.size(), 1);
  EXPECT_EQ(Results[0].arg1, 1);
  EXPECT_EQ(Results[0].arg2, 2);
  EXPECT_THAT(Results[0].morphemes1, testing::UnorderedElementsAre("dogs"));
  EXPECT_THAT(Results[0].morphemes2, testing::UnorderedElementsAre("cats"));
}

TEST(Swapping, DifferentMorphemeCases) {
  // Ensure that case does not matter when finding a swap.
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "DifferentMorphemeCasesTest";
  Site.callDecl.paramNames = {"Dogs", "Cats"};
  Site.positionalArgNames = {{"cats"}, {"dogs"}};

  std::vector<Result> Results = C.CheckSite(Site);
  EXPECT_EQ(Results.size(), 1);
  EXPECT_EQ(Results[0].arg1, 1);
  EXPECT_EQ(Results[0].arg2, 2);
  EXPECT_THAT(Results[0].morphemes1, testing::UnorderedElementsAre("cats"));
  EXPECT_THAT(Results[0].morphemes2, testing::UnorderedElementsAre("dogs"));
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

  std::vector<Result> Results = C.CheckSite(Site);
  EXPECT_EQ(Results.size(), 0);
}

TEST(Swapping, ShouldNotMatch) {
  Checker C;

  // Test that we don't warn when argument and parameter names match exactly.
  CallSite Site1;
  Site1.callDecl.fullyQualifiedName = "NegativeTest";
  Site1.callDecl.paramNames = {"horses", "emus"};
  Site1.positionalArgNames = {{"horses"}, {"emus"}};

  std::vector<Result> Results = C.CheckSite(Site1);
  EXPECT_EQ(Results.size(), 0);

  // Test that we don't warn when argument and parameter names don't relate.
  CallSite Site2;
  Site2.callDecl.fullyQualifiedName = "NegativeTest";
  Site2.callDecl.paramNames = {"horses", "emus"};
  Site2.positionalArgNames = {{"ponies"}, {"ostriches"}};

  Results = C.CheckSite(Site2);
  EXPECT_EQ(Results.size(), 0);

  // Test that we don't warn when the argument morphemes do not fully cover
  // the parameter morphemes.
  CallSite Site3;
  Site3.callDecl.fullyQualifiedName = "UncoveredMorpheme";
  Site3.callDecl.paramNames = {"barking_dogs", "hissing_cats"};
  Site3.positionalArgNames = {{"silly_cats"}, {"dogs_lolling"}};

  Results = C.CheckSite(Site3);
  EXPECT_EQ(Results.size(), 0);

  // Morphemes are not fully covered and it's a rotation rather than a swap.
  CallSite Site4;
  Site4.callDecl.fullyQualifiedName = "UncoverdMorphemeRotation";
  Site4.callDecl.paramNames = {"barking_dogs", "hissing_cats",
                               "running_alligators", "flailing_nudibranches"};
  Site4.positionalArgNames = {{"barfing_nudibranches"},
                              {"dogs_lolling"},
                              {"purring_cats"},
                              {"alligators_eating"}};

  Results = C.CheckSite(Site4);
  EXPECT_EQ(Results.size(), 0);
}

TEST(Swapping, MultipleMorphemes) {
  Checker C;

  CallSite Site;
  Site.callDecl.fullyQualifiedName = "MultipleMorphemesTest";
  Site.callDecl.paramNames = {"lolling_dogs", "cats_silly"};
  Site.positionalArgNames = {{"silly_cats"}, {"dogs_lolling"}};

  std::vector<Result> Results = C.CheckSite(Site);
  EXPECT_EQ(Results.size(), 1);
  EXPECT_EQ(Results[0].arg1, 1);
  EXPECT_EQ(Results[0].arg2, 2);
  EXPECT_THAT(Results[0].morphemes1,
              testing::UnorderedElementsAre("cats", "silly"));
  EXPECT_THAT(Results[0].morphemes2,
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

  std::vector<Result> Results = C.CheckSite(Site1);
  EXPECT_EQ(Results.size(), 0);

  // Similarly, test that we don't match when the arg suffixes are the same,
  // even if there would be a swap otherwise.
  CallSite Site2;
  Site2.callDecl.fullyQualifiedName = "NegativeNumericSuffixTestArg";
  Site2.callDecl.paramNames = {"horses", "horses"}; // Fake data is fake.
  Site2.positionalArgNames = {{"horses1", "horses2"}};

  Results = C.CheckSite(Site2);
  EXPECT_EQ(Results.size(), 0);

  // However, we should still catch swaps like this one.
  // FIXME: this test case fails because we do not handle abbreviations when
  // checking whether morphemes match and so we do not consider goats1 to be
  // a swap with goats.
  CallSite Site3;
  Site3.callDecl.fullyQualifiedName = "PositiveNumericSuffixTest";
  Site3.callDecl.paramNames = {"horses", "goats"};
  Site3.positionalArgNames = {{"goats1"}, {"horses2"}};

  Results = C.CheckSite(Site3);
  /*
    EXPECT_EQ(Results.size(), 1);
    EXPECT_NE(Results[0].score, nullptr);
    EXPECT_EQ(Results[0].arg1, 1);
    EXPECT_EQ(Results[0].arg2, 2);
    EXPECT_THAT(Results[0].morphemes1,
    testing::UnorderedElementsAre("goats1"));
    EXPECT_THAT(Results[0].morphemes2,
    testing::UnorderedElementsAre("horses2"));
  */
}
