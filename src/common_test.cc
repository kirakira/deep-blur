#include <functional>
#include <iostream>
#include <string>

#include "common.h"

using namespace std;

template <typename Function>
bool RunTest(Function f, const string& test_name) {
  if (!f()) {
    cerr << "Failed: " << test_name << endl;
    return false;
  }
  return true;
}

namespace blur {

constexpr int ArrayGenerator(size_t t) {
  return (static_cast<int>(t) % 3) * 5 + 2;
}

bool TestGenerateArray() {
  constexpr auto a = GenerateArray<int, 5>(ArrayGenerator);
  static_assert(a.size() == 5, "Size of array is not 5.");
  static_assert(a[0] == ArrayGenerator(0), "Array content mismatch.");
  static_assert(a[1] == ArrayGenerator(1), "Array content mismatch.");
  static_assert(a[2] == ArrayGenerator(2), "Array content mismatch.");
  static_assert(a[3] == ArrayGenerator(3), "Array content mismatch.");
  static_assert(a[4] == ArrayGenerator(4), "Array content mismatch.");
  return true;
}

constexpr int Operation(int x, int y) { return x + y; }

bool TestAggregate() {
  static_assert(Aggregate(Operation, 1, 2, 3, 4, 5) == 15, "Wrong Aggregate.");
  return true;
}

bool TestFillBits() {
  static_assert(FillBits(0, 1, 63, 17) == 0x8000000000020003ULL,
                "Wrong FillBits.");
  return true;
}

constexpr auto Sum(int x, int y) { return x + y; }

bool TestCurry() {
  constexpr auto f = CurryFront(Sum, 5);
  constexpr auto z = f(9);
  static_assert(z == 14, "Curry failed");
  return true;
}

constexpr int Ref(int& x) { return ++x; }

bool TestCurryRef() {
  int x = 0;
  auto f = CurryFront(Ref, x);
  f();
  return x == 0;
}

bool TestCurryRefWrapper() {
  int x = 0;
  auto f = CurryFront(Ref, std::ref(x));
  f();
  return x == 1;
}

void NormalFunction(int) {}

bool TestCurryNonConstExpr() {
  auto f = CurryFront(NormalFunction, 5);
  f();
  return true;
}

bool TestLsb() {
  return lsb(12) == 2 && lsb(1) == 0 && lsb(96) == 5;
}

}  // namespace blur

int main() {
  bool fail = false;
  fail = !RunTest(blur::TestGenerateArray, "TestGenerateArray") || fail;
  fail = !RunTest(blur::TestAggregate, "TestAggregate") || fail;
  fail = !RunTest(blur::TestFillBits, "TestAggregate") || fail;
  fail = !RunTest(blur::TestCurry, "TestCurry") || fail;
  fail = !RunTest(blur::TestCurryRef, "TestCurryRef") || fail;
  fail = !RunTest(blur::TestCurryRefWrapper, "TestCurryRefWrapper") || fail;
  fail = !RunTest(blur::TestLsb, "TestLsb") || fail;
  if (fail) {
    cerr << "Some test failed." << endl;
  } else {
    cerr << "Success." << endl;
  }
  return fail ? 1 : 0;
}
