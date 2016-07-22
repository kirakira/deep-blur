#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "board.h"

using namespace blur;
using namespace std;

bool TestSetBoard() {
  Board board;
  if (board.ToString() !=
      "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR")
    return false;
  if (!board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != "4k4/9/9/9/9/9/9/9/9/4K4") return false;
  if (!board.SetBoard(
          "2eakae1r/5r3/hc4hc1/p1p1p1p1p/9/9/P1P1P1P1P/HC4HC1/3R4R/2EAKAE2"))
    return false;
  if (board.ToString() !=
      "2eakae1r/5r3/hc4hc1/p1p1p1p1p/9/9/P1P1P1P1P/HC4HC1/3R4R/2EAKAE2")
    return false;
  return true;
}

bool TestSetBadBoard() {
  Board board;
  const string board_fen =
      "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR";
  if (board.SetBoard("9/9/9/9/9/9/9/9/9/9")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4K4/9/9/9/9/9/9/9/9/4k4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k49/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4r/9/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4/")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4/9")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k3t/9/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4r")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4kk3/9/9/9/9/9/9/9/9/4K3r")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4/9/9/9/9/9/9/9/rr7/4K3r")) return false;
  if (board.ToString() != board_fen) return false;
  return true;
}

bool TestGenerateMoves() {
  Board board;
  auto moves = board.GenerateMoves(Side::kRed);
  vector<string> moves_string;
  for (auto move : moves) {
    moves_string.push_back(move.ToString());
  }
  std::sort(moves_string.begin(), moves_string.end());
  if (moves_string !=
      vector<string>{"a0a1", "a0a2", "a3a4", "b0a2", "b0c2", "b2a2", "b2b1",
                     "b2b3", "b2b4", "b2b5", "b2b6", "b2b9", "b2c2", "b2d2",
                     "b2e2", "b2f2", "b2g2", "c0a2", "c0e2", "c3c4", "d0e1",
                     "e0e1", "e3e4", "f0e1", "g0e2", "g0i2", "g3g4", "h0g2",
                     "h0i2", "h2c2", "h2d2", "h2e2", "h2f2", "h2g2", "h2h1",
                     "h2h3", "h2h4", "h2h5", "h2h6", "h2h9", "h2i2", "i0i1",
                     "i0i2", "i3i4"}) {
    return false;
  }
  if (board.IsAttacked(Position("a0")).first) return false;
  if (board.IsAttacked(Position("b0")) != make_pair(true, Position("b7"))) {
    return false;
  }
  return true;
}

bool TestIsAttacked() {
  Board board;
  if (!board.SetBoard("2eakae2/3P5/9/9/9/9/9/9/9/4K4")) return false;
  if (board.IsAttacked(Position("d8")).first) return false;
  if (board.IsAttacked(Position("e0")) != make_pair(true, Position("e9"))) {
    return false;
  }
  return true;
}

bool TestMake() {
  Board board;
  board.Make(Move("b2b9"));
  if (board.ToString() !=
      "rCeakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEHR") {
    return false;
  }
  board.Make(Move("h7h0"));
  if (board.ToString() !=
      "rCeakaehr/9/1c7/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEcR") {
    return false;
  }
  board.Unmake();
  if (board.ToString() !=
      "rCeakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEHR") {
    return false;
  }
  board.Unmake();
  if (board.ToString() !=
      "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR") {
    return false;
  }
  return true;
}

bool TestInCheck() {
  Board board;
  if (!board.SetBoard("2eakae2/3P5/9/9/9/9/9/9/9/4K4")) return false;
  if (!board.InCheck(Side::kRed)) return false;
  if (!board.InCheck(Side::kBlack)) return false;
  if (!board.SetBoard(
          "rheakaehr/9/1c5c1/p1p1p1p1p/9/4C4/P1P1P1P1P/1C7/9/RHEAKAEHR")) {
    return false;
  }
  if (board.InCheck(Side::kRed)) return false;
  if (!board.InCheck(Side::kBlack)) return false;
  return true;
}

bool TestCheckedMoveForBoard(Board board) {
  static constexpr std::initializer_list<Side> all_sides{Side::kRed,
                                                         Side::kBlack};
  for (auto s : all_sides) {
    auto moves = board.GenerateMoves(s);
    for (int i = 0; i < kNumPositions; ++i) {
      for (int j = 0; j < kNumPositions; ++j) {
        Move move{Position(i), Position(j)};
        bool valid = moves.end() !=
                     std::find_if(moves.begin(), moves.end(), [move](Move m) {
                       return move.from() == m.from() && move.to() == m.to();
                     });
        if (valid) {
          board.Make(move);
          if (board.InCheck(s)) valid = false;
          board.Unmake();
        }
        if (board.CheckedMake(s, move)) {
          if (!valid) return false;
          board.Unmake();
        } else {
          if (valid) return false;
        }
      }
    }
  }
  return true;
}

bool TestCheckedMove() {
  Board board;
  cout << "A" << endl;
  if (!TestCheckedMoveForBoard(board)) return false;
  cout << "B" << endl;
  if (!board.SetBoard("2eakae2/3P5/9/9/9/9/9/9/9/4K4")) return false;
  cout << "C" << endl;
  if (!TestCheckedMoveForBoard(board)) return false;
  cout << "D" << endl;
  if (!board.SetBoard(
          "rCeakaehr/9/1c7/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEcR")) {
    return false;
  }
  cout << "E" << endl;
  if (!TestCheckedMoveForBoard(board)) return false;
  cout << "F" << endl;
  return true;
}

int main() {
  bool success = true;
  success = success && TestSetBoard();
  success = success && TestSetBadBoard();
  success = success && TestGenerateMoves();
  success = success && TestIsAttacked();
  success = success && TestMake();
  success = success && TestInCheck();
  success = success && TestCheckedMove();
  cout << (success ? "Success." : "Failed.") << endl;
  return success ? 0 : 1;
}
