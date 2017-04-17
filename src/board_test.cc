#include <algorithm>
#include <iostream>
#include <set>
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

  moves_string.clear();
  moves = board.GenerateCaptures(Side::kRed);
  for (auto move : moves) {
    moves_string.push_back(move.ToString());
  }
  std::sort(moves_string.begin(), moves_string.end());
  if (moves_string != vector<string>{"b2b9", "h2h9"}) {
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
  set<uint64> hashes;
  Board board;
  const auto hash00 = board.HashCode(Side::kRed),
             hash01 = board.HashCode(Side::kBlack);
  hashes.insert(hash00);
  hashes.insert(hash01);

  board.Make(Move("b2b9"));
  if (board.ToString() !=
      "rCeakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEHR") {
    return false;
  }
  const auto hash10 = board.HashCode(Side::kRed),
             hash11 = board.HashCode(Side::kBlack);
  hashes.insert(hash10);
  hashes.insert(hash11);

  board.Make(Move("h7h0"));
  if (board.ToString() !=
      "rCeakaehr/9/1c7/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEcR") {
    return false;
  }
  const auto hash20 = board.HashCode(Side::kRed),
             hash21 = board.HashCode(Side::kBlack);
  hashes.insert(hash20);
  hashes.insert(hash21);

  if (hashes.size() != 6) return false;

  board.Unmake();
  if (board.ToString() !=
      "rCeakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEHR") {
    return false;
  }
  if (board.HashCode(Side::kRed) != hash10) return false;
  if (board.HashCode(Side::kBlack) != hash11) return false;

  board.Unmake();
  if (board.ToString() !=
      "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR") {
    return false;
  }
  if (board.HashCode(Side::kRed) != hash00) return false;
  if (board.HashCode(Side::kBlack) != hash01) return false;

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
        if (board.CheckedMake(s, move).first) {
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

bool TestCheckedMake() {
  Board board;
  if (!TestCheckedMoveForBoard(board)) return false;
  if (!board.SetBoard("2eakae2/3P5/9/9/9/9/9/9/9/4K4")) return false;
  if (!TestCheckedMoveForBoard(board)) return false;
  if (!board.SetBoard(
          "rCeakaehr/9/1c7/p1p1p1p1p/9/9/P1P1P1P1P/7C1/9/RHEAKAEcR")) {
    return false;
  }
  if (!TestCheckedMoveForBoard(board)) return false;
  return true;
}

bool TestCheckedUnmake() {
  Board board;
  if (board.CheckedUnmake()) return false;
  board.Make(Move("b2c2"));
  if (!board.CheckedUnmake()) return false;
  if (board.ToString() !=
      "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR") {
    return false;
  }
  return true;
}

bool VerifyRepetitionSequence(const string& board_string,
                              const vector<string>& moves, MoveType expected) {
  Board board;
  if (!board.SetBoard(board_string)) return false;
  for (size_t i = 0; i < moves.size(); ++i) {
    if (moves[i] == "xr") {
      board.ResetRepetitionHistory(Side::kRed);
    } else if (moves[i] == "xb") {
      board.ResetRepetitionHistory(Side::kBlack);
    } else if (moves[i] == "u") {
      board.Unmake();
    } else {
      const auto result = board.Make(Move(moves[i]));
      if (i + 1 == moves.size()) {
        if (result != expected) {
          return false;
        }
      } else {
        if (result != MoveType::kRegular) {
          return false;
        }
      }
    }
  }
  return true;
}

bool TestRepetition() {
  if (!VerifyRepetitionSequence("3k5/9/3R5/9/9/9/9/9/9/5K3",
                                {"d9e9", "d7e7", "e9d9", "e7d7"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("3k5/9/4R4/9/9/9/9/9/9/5K3",
                                {"e7d7", "d9e9", "d7e7", "e9d9"},
                                MoveType::kPerpetualAttackee)) {
    return false;
  }
  if (!VerifyRepetitionSequence("3k5/9/4R4/9/9/9/9/9/9/5K3",
                                {"e7f7", "d9e9", "f7e7", "e9d9"},
                                MoveType::kRepetition)) {
    return false;
  }
  if (!VerifyRepetitionSequence(
          "C1ck5/1R7/9/9/9/9/9/9/9/5K3",
          {"c9c8", "b8b9", "c8c9", "b9b7", "c9c8", "b7b9"},
          MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("C1ck5/1R7/9/9/9/9/9/9/9/5K3",
                                {"c9c8", "xr", "b8b9", "c8c9", "b9b8", "c9c8"},
                                MoveType::kPerpetualAttackee)) {
    return false;
  }
  if (!VerifyRepetitionSequence("C1ck5/1R7/9/9/9/9/9/9/9/5K3",
                                {"c9c8", "xb", "b8b9", "c8c9", "b9b8"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("2Rk5/9/9/9/9/9/9/9/9/5K3", {"d9d8", "d8d9"},
                                MoveType::kRepetition)) {
    return false;
  }
  if (!VerifyRepetitionSequence("1cR6/3k5/9/9/9/9/9/9/9/5K3",
                                {"c9b9", "u", "c9c8", "d8d9", "c8c9", "d9d8"},
                                MoveType::kPerpetualAttackee)) {
    return false;
  }
  if (!VerifyRepetitionSequence("r2k5/9/9/p1p6/9/C8/9/9/9/4K4",
                                {"a9c9", "a4c4", "c9a9", "c4a4"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("r2k5/9/9/p1p6/C8/9/9/9/9/4K4",
                                {"a9c9", "a5c5", "c9a9", "c5a5"},
                                MoveType::kRepetition)) {
    return false;
  }
  if (!VerifyRepetitionSequence("r2k5/9/1H7/9/9/9/9/9/9/4K4",
                                {"a9a8", "b7c9", "a8a9", "c9b7"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("1r2k4/9/2H6/9/9/9/9/9/9/5K3",
                                {"b9b8", "c7d9", "b8b9", "d9c7"},
                                MoveType::kRepetition)) {
    return false;
  }
  if (!VerifyRepetitionSequence("h2k5/9/2R6/9/9/9/9/9/9/4K4",
                                {"c7a7", "a9c8", "a7c7", "c8a9"},
                                MoveType::kPerpetualAttackee)) {
    return false;
  }
  if (!VerifyRepetitionSequence("c2k5/9/2R6/9/9/9/9/9/9/4K4",
                                {"c7a7", "a9c9", "a7c7", "c9a9"},
                                MoveType::kPerpetualAttackee)) {
    return false;
  }
  if (!VerifyRepetitionSequence("r2k5/5C3/1H7/9/9/9/9/9/9/4K4",
                                {"a9a8", "b7c9", "a8a9", "c9b7"},
                                MoveType::kRepetition)) {
    return false;
  }
  if (!VerifyRepetitionSequence("r2k5/3C5/1H7/9/9/9/9/9/9/4K4",
                                {"a9a8", "b7c9", "a8a9", "c9b7"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("C1hk5/9/8r/9/9/9/9/9/9/5K3",
                                {"c9d7", "a9a7", "d7c9", "a7a9"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("C1hk5/9/8h/9/9/9/9/9/9/5K3",
                                {"c9d7", "a9a7", "d7c9", "a7a9"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("C1hk5/9/8h/8r/9/9/9/9/9/5K3",
                                {"c9d7", "a9a7", "d7c9", "a7a9"},
                                MoveType::kRepetition)) {
    return false;
  }
  if (!VerifyRepetitionSequence("c4k3/9/1H7/9/9/9/9/9/9/3K5",
                                {"a9a8", "b7c9", "a8a9", "c9b7"},
                                MoveType::kPerpetualAttacker)) {
    return false;
  }
  if (!VerifyRepetitionSequence("c4k2c/9/1H7/9/9/9/9/9/9/3K5",
                                {"a9a8", "b7c9", "a8a9", "c9b7"},
                                MoveType::kRepetition)) {
    return false;
  }
  return true;
}

bool TestValidMove() {
  Move move(0);
  if (move.IsValid()) return false;
  move = Move(91);
  if (move.IsValid()) return false;
  move = Move(23296);
  if (move.IsValid()) return false;
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
  success = success && TestCheckedMake();
  success = success && TestCheckedUnmake();
  success = success && TestRepetition();
  success = success && TestValidMove();
  cout << (success ? "Success." : "Failed.") << endl;
  return success ? 0 : 1;
}
