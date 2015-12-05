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

constexpr int ArrayGenerator(size_t t) { return (t % 3) * 5 + 2; }

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
  static_assert(Aggregate(Operation, 1, 2, 3, 4, 5) == 15, "Wrong aggregate.");
  return true;
}

}  // namespace blur

int main() {
  bool fail = false;
  fail = !RunTest(blur::TestGenerateArray, "TestGenerateArray") || fail;
  if (fail) {
    cerr << "Some test failed." << endl;
  } else {
    cerr << "Success." << endl;
  }
  return fail ? 1 : 0;
}
