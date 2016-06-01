#include "bitboard.h"

#include <type_traits>
#include <vector>

using std::vector;

namespace blur {

uint64 TestFunctionHa(BitBoard board) {
  uint64 ans = 0;
  for (auto pos : board) {
    ans ^= pos.value();
  }
  return ans;
}

template <typename ContainerType>
auto DumpElements(const ContainerType& container) {
  using value_type = std::decay_t<decltype(*container.begin())>;
  vector<value_type> dumped;
  for (const auto& value : container) {
    dumped.push_back(value);
  }
  return dumped;
}

bool TestIterateHalfBitBoard() {
  auto board = HalfBitBoard::EmptyBoard();
  if (DumpElements(board).size() != 0) return false;
  for (auto i : {0, 6, 7, 20, 44}) {
    board |= HalfBitBoard::Fill(Position(i));
  }
  if (DumpElements(board) != vector<Position>{Position(0), Position(6),
                                              Position(7), Position(20),
                                              Position(44)}) {
    return false;
  }
  return true;
}

bool TestIterateBitBoard1() {
  auto board = BitBoard::EmptyBoard();
  if (DumpElements(board).size() != 0) return false;
  for (auto i : {0, 6, 7, 20, 44, 45, 67, 88, 89}) {
    board |= BitBoard::Fill(Position(i));
  }
  if (DumpElements(board) !=
      vector<Position>{Position(0), Position(6), Position(7), Position(20),
                       Position(44), Position(45), Position(67), Position(88),
                       Position(89)}) {
    return false;
  }
  return true;
}

bool TestIterateBitBoard2() {
  auto board = BitBoard::EmptyBoard();
  if (DumpElements(board).size() != 0) return false;
  for (auto i : {0, 6, 7, 20, 44}) {
    board |= BitBoard::Fill(Position(i));
  }
  if (DumpElements(board) != vector<Position>{Position(0), Position(6),
                                              Position(7), Position(20),
                                              Position(44)}) {
    return false;
  }
  return true;
}

bool TestIterateBitBoard3() {
  auto board = BitBoard::EmptyBoard();
  if (DumpElements(board).size() != 0) return false;
  for (auto i : {45, 67, 88, 89}) {
    board |= BitBoard::Fill(Position(i));
  }
  if (DumpElements(board) != vector<Position>{Position(45), Position(67),
                                              Position(88), Position(89)}) {
    return false;
  }
  return true;
}

}  // namespace blur

int main() {
  using namespace blur;

  bool success = true;
  success = success && TestIterateHalfBitBoard();
  success = success && TestIterateBitBoard1();
  success = success && TestIterateBitBoard2();
  success = success && TestIterateBitBoard3();
  if (success) {
    std::cout << "Success." << std::endl;
  } else {
    std::cout << "Failed." << std::endl;
  }
  return success ? 0 : 1;
}
