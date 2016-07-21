#include "bitboard.h"

#include <type_traits>
#include <vector>

using std::vector;

namespace blur {

template <typename BoardType>
auto DumpPositions(BoardType board) {
  vector<Position> dumped;
  for (auto iter = board.Positions(); iter.HasNext();) {
    dumped.push_back(iter.Next());
  }
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

}  // namespace blur

int main() {
  using namespace blur;

  bool success = true;
  success = success && TestIterateHalfBitBoard();
  success = success && TestIterateBitBoard1();
  success = success && TestIterateBitBoard2();
  success = success && TestIterateBitBoard3();
  success = success && TestMake();
  if (success) {
    std::cout << "Success." << std::endl;
  } else {
    std::cout << "Failed." << std::endl;
  }
  return success ? 0 : 1;
}
