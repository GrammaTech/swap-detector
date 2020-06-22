// RUN:  %clang_analyze_cc1 -load %llvmshlibdir/SwapDetectorPlugin%shlibext -Wno-gnu-alignof-expression -analyzer-checker=gt.ExprNames %s -verify
// REQUIRES: plugins

template <typename T> void clang_analyzer_exprName(T &&);

int fork();
extern "C" int *__error();
#define errno (*__error())
#define WEXITSTATUS(x) x
#define O_CREAT 512
#define O_APPEND 256

struct TestStruct {
  int member;
  void main();
  static int staticFunc();
};

void TestStruct::main() {
  int local;
  char localArr[3];
  TestStruct localStruct;
  auto fnPtr = &fork;

  clang_analyzer_exprName(O_CREAT); // expected-warning{{O_CREAT}}
  clang_analyzer_exprName(O_CREAT | O_APPEND); // no-warning
  clang_analyzer_exprName(errno); // expected-warning{{errno}}
  clang_analyzer_exprName(WEXITSTATUS(3)); // expected-warning{{WEXITSTATUS}}
  clang_analyzer_exprName(fork()); // expected-warning{{fork}}
  clang_analyzer_exprName(&local); // expected-warning{{local}}
  clang_analyzer_exprName((char *)&local); // expected-warning{{local}}
  clang_analyzer_exprName(sizeof(local)); // expected-warning{{sizeof}}
  clang_analyzer_exprName(sizeof(int)); // expected-warning{{sizeof}}
  clang_analyzer_exprName(alignof(local)); // expected-warning{{alignof}}
  clang_analyzer_exprName(alignof(int)); // expected-warning{{alignof}}
  clang_analyzer_exprName(localArr[1]); // expected-warning{{localArr}}
  clang_analyzer_exprName(this->member); // expected-warning{{member}}
  clang_analyzer_exprName(localStruct.member); // expected-warning{{member}}
  clang_analyzer_exprName(this); // expected-warning{{this}}
  clang_analyzer_exprName(3); // expected-warning{{3}}
  clang_analyzer_exprName(3 + 4); // no-warning
  clang_analyzer_exprName((3 + 4)); // no-warning
  clang_analyzer_exprName(3.5f); // expected-warning{{3.5F}}
  clang_analyzer_exprName('a'); // expected-warning{{'a'}}
  clang_analyzer_exprName("hello"); // expected-warning{{"hello"}}
  clang_analyzer_exprName(true); // expected-warning{{true}}
  clang_analyzer_exprName(nullptr); // expected-warning{{nullptr}}
  clang_analyzer_exprName(1i); // expected-warning{{1i}}
  clang_analyzer_exprName(fnPtr()); // expected-warning{{fnPtr}}
  clang_analyzer_exprName(TestStruct::staticFunc()); // expected-warning{{staticFunc}}
  clang_analyzer_exprName(reinterpret_cast<char (*)(void)>(4096)()); // expected-warning{{4096}}
}
