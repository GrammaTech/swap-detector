// RUN: %clang_analyze_cc1 -load %llvmshlibdir/SwappedArgPlugin%shlibext -analyzer-checker=gt.SwappedArgs -analyzer-config gt.SwappedArgs:ModelPath=%S/test.db -verify %s
// REQUIRES: plugins

void func(int cats, int dogs);

int main(void) {
  int dogs = 1, cats = 2;
  func(dogs, cats); // expected-warning {{arguments 1 and 2 are swapped with morpheme1 = dogs and morpheme2 = cats}}
}

