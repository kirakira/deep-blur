#include <iostream>
#include <string>

#include "board.h"

using namespace blur;
using namespace std;

void PrintPawnTables() {
  cout << "Red" << endl;
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << BitTables::red_pawn_moves[i] << endl;
  }
  cout << "Black" << endl;
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << BitTables::black_pawn_moves[i] << endl;
  }
}

void PrintKingTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << BitTables::king_moves[i] << endl;
  }
}

void PrintAssistantTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << BitTables::assistant_moves[i] << endl;
  }
}

void PrintElephantTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    for (uint64 occ = 0; occ < (1 << 4); ++occ)
      cout << "Pos: " << i << ", occupancy: " << occ << endl
           << BitTables::elephant_moves[i][occ] << endl;
  }
}

void PrintHorseTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    for (uint64 occ = 0; occ < (1 << 4); ++occ) {
      string occ_str;
      if (GetBit(occ, 0)) occ_str += "L";
      if (GetBit(occ, 1)) occ_str += "B";
      if (GetBit(occ, 2)) occ_str += "R";
      if (GetBit(occ, 3)) occ_str += "T";
      if (occ_str == "") occ_str = "NONE";
      cout << "Pos: " << i << ", occupancy: " << occ_str << endl
           << BitTables::horse_moves[i][occ] << endl;
    }
  }
}

bool CheckHorseTables() {
  auto board = BitBoard::Fill(Position(49 + 9));
  uint64 occ = board.GetHorseOccupancy(Position(49));
  uint64 expected = 1 << 3;
  return expected == occ;
}

bool CheckCannonRowMovesTables() {
  auto board = BitBoard::Fill(Position(5, 0)) | BitBoard::Fill(Position(5, 1)) |
               BitBoard::Fill(Position(5, 5)) | BitBoard::Fill(Position(5, 7));
  auto moves =
      BitTables::cannon_row_moves[Position(5, 1)
                                      .value()][board.GetRowOccupancy(5)];
  auto expected_moves =
      BitBoard::Fill(Position(5, 2)) | BitBoard::Fill(Position(5, 3)) |
      BitBoard::Fill(Position(5, 4)) | BitBoard::Fill(Position(5, 7));
  return moves == expected_moves;
}

bool TestColOccupancy() {
  for (int col = 0; col < kNumColumns; ++col) {
    for (uint64 x = 0; x < (1 << kNumRows); ++x) {
      auto board = BitBoard::EmptyBoard();
      for (int pos = 0; pos < kNumPositions; ++pos) {
        Position p(pos);
        if (p.Column() != col) board |= BitBoard::Fill(p);
      }
      for (int i = 0; i < kNumRows; ++i) {
        if (GetBit(x, i)) {
          board |= BitBoard::Fill(Position(i, col));
        }
      }
      auto occ = board.GetColOccupancy(col);
      if (occ != x) {
        cout << "Expected: " << x << ", received: " << occ << endl;
        return false;
      }
    }
  }
  return true;
}

bool CheckCannonColMovesTables() {
  auto board = BitBoard::Fill(Position(5, 1)) | BitBoard::Fill(Position(6, 1)) |
               BitBoard::Fill(Position(7, 1)) | BitBoard::Fill(Position(9, 1)) |
               BitBoard::Fill(Position(3, 1)) | BitBoard::Fill(Position(1, 1));
  auto moves = BitTables::cannon_col_moves[Position(5, 1).value()]
                                          [board.GetColOccupancy(1)];

  auto expected_moves = BitBoard::Fill(Position(7, 1)) |
                        BitBoard::Fill(Position(4, 1)) |
                        BitBoard::Fill(Position(1, 1));
  return moves == expected_moves;
}

int main() {
  bool success = true;
  success = success && CheckHorseTables();
  success = success && CheckCannonRowMovesTables();
  success = success && TestColOccupancy();
  success = success && CheckCannonColMovesTables();
  cout << (success ? "Success." : "Failed.") << endl;
  return success ? 0 : 1;
}
