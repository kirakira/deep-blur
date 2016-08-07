#include "bitboard.h"

#include <type_traits>
#include <vector>

using std::vector;

namespace blur {

template <typename BoardType>
auto DumpPositions(BoardType board) {
  vector<Position> dumped;
  board.VisitPositions([&dumped](Position pos) { dumped.push_back(pos); });
  return dumped;
}

bool TestIterateHalfBitBoard() {
  auto board = HalfBitBoard::EmptyBoard();
  if (DumpPositions(board).size() != 0) return false;
  for (auto i : {0, 6, 7, 20, 44}) {
    board |= HalfBitBoard::Fill(Position(i));
  }
  if (DumpPositions(board) != vector<Position>{Position(0), Position(6),
                                               Position(7), Position(20),
                                               Position(44)}) {
    return false;
  }
  return true;
}

bool TestIterateBitBoard1() {
  auto board = BitBoard::EmptyBoard();
  if (DumpPositions(board).size() != 0) return false;
  for (auto i : {0, 6, 7, 20, 44, 45, 67, 88, 89}) {
    board |= BitBoard::Fill(Position(i));
  }
  if (DumpPositions(board) !=
      vector<Position>{Position(0), Position(6), Position(7), Position(20),
                       Position(44), Position(45), Position(67), Position(88),
                       Position(89)}) {
    return false;
  }
  return true;
}

bool TestIterateBitBoard2() {
  auto board = BitBoard::EmptyBoard();
  if (DumpPositions(board).size() != 0) return false;
  for (auto i : {0, 6, 7, 20, 44}) {
    board |= BitBoard::Fill(Position(i));
  }
  if (DumpPositions(board) != vector<Position>{Position(0), Position(6),
                                               Position(7), Position(20),
                                               Position(44)}) {
    return false;
  }
  return true;
}

bool TestIterateBitBoard3() {
  auto board = BitBoard::EmptyBoard();
  if (DumpPositions(board).size() != 0) return false;
  for (auto i : {45, 67, 88, 89}) {
    board |= BitBoard::Fill(Position(i));
  }
  if (DumpPositions(board) != vector<Position>{Position(45), Position(67),
                                               Position(88), Position(89)}) {
    return false;
  }
  return true;
}

bool TestMake() {
  auto board = BitBoard::Fill(Position(0)) | BitBoard::Fill(Position(23)) |
               BitBoard::Fill(Position(57)) | BitBoard::Fill(Position(87));
  Move move(Position(23), Position(16));
  board.Make(move);
  if (DumpPositions(board) !=
      vector<Position>{Position(0), Position(16), Position(57), Position(87)}) {
    return false;
  }
  board.Unmake(move);
  if (DumpPositions(board) !=
      vector<Position>{Position(0), Position(23), Position(57), Position(87)}) {
    return false;
  }
  move = Move(Position(0), Position(45));
  board.Make(move);
  if (DumpPositions(board) != vector<Position>{Position(23), Position(45),
                                               Position(57), Position(87)}) {
    return false;
  }
  move = Move(Position(87), Position(89));
  board.Make(move);
  if (DumpPositions(board) != vector<Position>{Position(23), Position(45),
                                               Position(57), Position(89)}) {
    return false;
  }
  move = Move(Position(89), Position(0));
  board.Make(move);
  if (DumpPositions(board) !=
      vector<Position>{Position(0), Position(23), Position(45), Position(57)}) {
    return false;
  }
  board.Unmake(move);
  if (DumpPositions(board) != vector<Position>{Position(23), Position(45),
                                               Position(57), Position(89)}) {
    return false;
  }
  return true;
}

bool TestShiftLeft() {
  vector<int> test_pos = {0, 27, 28, 44, 45, 79, 80, 89};

  const auto get_board = [](const vector<int>& pos) {
    BitBoard b = BitBoard::EmptyBoard();
    for (int p : pos) b |= BitBoard::Fill(Position(p));
    return b;
  };
  const auto shift_vector = [](const vector<int>& pos, int delta) {
    vector<int> ans;
    for (int p : pos) {
      if (p + delta >= 0 && p + delta < kNumPositions) {
        ans.push_back(p + delta);
      }
    }
    return ans;
  };

  const BitBoard b = get_board(test_pos);
  for (int x = 0; x < kNumPositions; ++x) {
    if ((b << x) != get_board(shift_vector(test_pos, x))) return false;
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
  success = success && TestMake();
  success = success && TestShiftLeft();
  if (success) {
    std::cout << "Success." << std::endl;
  } else {
    std::cout << "Failed." << std::endl;
  }
  return success ? 0 : 1;
}
