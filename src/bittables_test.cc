#include <iostream>
#include <string>
#include <vector>

#include "bittables.h"

using namespace blur;
using namespace std;

namespace {

constexpr BitTables kBitTables{};

}  // namespace

void PrintPawnTables() {
  cout << "Red" << endl;
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << kBitTables.red_pawn_moves[i] << endl;
  }
  cout << "Black" << endl;
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << kBitTables.black_pawn_moves[i] << endl;
  }
}

bool TestPawnReverseTables() {
  if (kBitTables.red_pawn_reverse_moves[36] != BitBoard::Fill(Position(27)))
    return false;
  if (kBitTables.red_pawn_reverse_moves[45] !=
      (BitBoard::Fill(Position(36)) | BitBoard::Fill(Position(46))))
    return false;
  if (kBitTables.black_pawn_reverse_moves[36] !=
      (BitBoard::Fill(Position(45)) | BitBoard::Fill(Position(37))))
    return false;
  if (kBitTables.black_pawn_reverse_moves[45] != BitBoard::Fill(Position(54)))
    return false;
  return true;
}

void PrintKingTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << kBitTables.king_moves[i] << endl;
  }
}

void PrintAssistantTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << kBitTables.assistant_moves[i] << endl;
  }
}

bool TestElephantOccupancy() {
  vector<int> elephant_positions = {2,  6,  18, 22, 26, 38, 42,
                                    47, 51, 63, 67, 71, 83, 87};
  for (int pos : elephant_positions) {
    Position p(pos);
    const int row = p.Row(), col = p.Column();
    for (uint64 x = 0; x < 16; ++x) {
      BitBoard board = BitBoard::EmptyBoard();
      for (int i = 0; i < kNumPositions; ++i) {
        Position ip(i);
        const int irow = ip.Row(), icol = ip.Column();
        if (irow == row - 1 && icol == col - 1) {
          if (GetBit(x, 0)) board |= BitBoard::Fill(ip);
        } else if (irow == row + 1 && icol == col - 1) {
          if (GetBit(x, 1)) board |= BitBoard::Fill(ip);
        } else if (irow == row - 1 && icol == col + 1) {
          if (GetBit(x, 2)) board |= BitBoard::Fill(ip);
        } else if (irow == row + 1 && icol == col + 1) {
          if (GetBit(x, 3)) board |= BitBoard::Fill(ip);
        } else {
          board |= BitBoard::Fill(ip);
        }
      }
      uint64 occ = board.GetElephantOccupancy(p);
      uint64 expected = 0;
      if (GetBit(x, 0) && row != 0 && row != 5 && col != 0) expected |= 1;
      if (GetBit(x, 1) && row != 4 && row != 9 && col != 0) expected |= 2;
      if (GetBit(x, 2) && row != 0 && row != 5 && col != 8) expected |= 4;
      if (GetBit(x, 3) && row != 4 && row != 9 && col != 8) expected |= 8;
      if (expected != occ) {
        cout << "(" << row << ", " << col << ")" << endl;
        cout << board << endl;
        cout << "expected: " << expected << "; received: " << occ << endl;
        return false;
      }
    }
  }
  return true;
}

void PrintElephantTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    for (uint64 occ = 0; occ < (1 << 4); ++occ)
      cout << "Pos: " << i << ", occupancy: " << occ << endl
           << kBitTables.elephant_moves[i][occ] << endl;
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
           << kBitTables.horse_moves[i][occ] << endl;
    }
  }
}

bool TestHorseOccupancy() {
  for (int pos = 0; pos < kNumPositions; ++pos) {
    Position p(pos);
    const int row = p.Row(), col = p.Column();

    for (uint64 x = 0; x < 16; ++x) {
      auto board = BitBoard::EmptyBoard();
      for (int i = 0; i < kNumPositions; ++i) {
        Position ip(i);
        const int irow = ip.Row(), icol = ip.Column();
        if (irow == row && icol == col - 1) {
          if (GetBit(x, 0)) board |= BitBoard::Fill(ip);
        } else if (irow == row - 1 && icol == col) {
          if (GetBit(x, 1)) board |= BitBoard::Fill(ip);
        } else if (irow == row && icol == col + 1) {
          if (GetBit(x, 2)) board |= BitBoard::Fill(ip);
        } else if (irow == row + 1 && icol == col) {
          if (GetBit(x, 3)) board |= BitBoard::Fill(ip);
        } else {
          board |= BitBoard::Fill(ip);
        }
      }
      uint64 occ = board.GetHorseOccupancy(p);
      uint64 expected = 0;
      if (GetBit(x, 0) && col > 0) expected |= 1;
      if (GetBit(x, 1) && row > 0) expected |= 2;
      if (GetBit(x, 2) && col < 8) expected |= 4;
      if (GetBit(x, 3) && row < 9) expected |= 8;
      if (occ != expected) {
        cout << "(" << row << ", " << col << ")" << endl;
        cout << board << endl;
        cout << "Expected: " << expected << ", got: " << occ << endl;
        return false;
      }
    }
  }
  return true;
}

bool CheckHorseTables() {
  auto board = BitBoard::Fill(Position(49 + 9));
  uint64 occ = board.GetHorseOccupancy(Position(49));
  uint64 expected = 1 << 3;
  return expected == occ;
}

bool CheckHorseReverseTables() {
  auto board = BitBoard::Fill(Position(46 + 10));
  uint64 occ = board.GetElephantOccupancy(Position(46));
  return kBitTables.horse_reverse_moves[46][occ] ==
         (BitBoard::Fill(Position(27)) | BitBoard::Fill(Position(29)) |
          BitBoard::Fill(Position(39)) | BitBoard::Fill(Position(63)));
}

bool TestRowOccupancy() {
  for (int row = 0; row < kNumRows; ++row) {
    for (int x = 0; x < (1 << kNumColumns); ++x) {
      auto board = BitBoard::EmptyBoard();
      for (int pos = 0; pos < kNumPositions; ++pos) {
        Position p(pos);
        if (p.Row() != row) board |= BitBoard::Fill(p);
      }
      for (int i = 0; i < kNumColumns; ++i) {
        if (GetBit(x, i)) board |= BitBoard::Fill(Position(row, i));
      }
      auto occ = board.GetRowOccupancy(row);
      if (occ != x) return false;
    }
  }
  return true;
}

bool CheckCannonRowTables() {
  auto board = BitBoard::Fill(Position(5, 0)) | BitBoard::Fill(Position(5, 1)) |
               BitBoard::Fill(Position(5, 5)) | BitBoard::Fill(Position(5, 7));
  auto moves =
      kBitTables
          .cannon_row_moves[1][board.GetRowOccupancy(5)];
  auto expected_moves =
      BitBoard::Fill(Position(0, 2)) | BitBoard::Fill(Position(0, 3)) |
      BitBoard::Fill(Position(0, 4)) | BitBoard::Fill(Position(0, 7));
  return moves == expected_moves;
}

bool TestColOccupancy() {
  for (int col = 0; col < kNumColumns; ++col) {
    for (int x = 0; x < (1 << kNumRows); ++x) {
      auto board = BitBoard::EmptyBoard();
      for (int pos = 0; pos < kNumPositions; ++pos) {
        Position p(pos);
        if (p.Column() != col) board |= BitBoard::Fill(p);
      }
      for (int i = 0; i < kNumRows; ++i) {
        if (GetBit(x, i)) board |= BitBoard::Fill(Position(i, col));
      }
      auto occ = board.GetColOccupancy(col);
      if (occ != x) return false;
    }
  }
  return true;
}

bool CheckCannonColTables() {
  auto board = BitBoard::Fill(Position(5, 1)) | BitBoard::Fill(Position(6, 1)) |
               BitBoard::Fill(Position(7, 1)) | BitBoard::Fill(Position(9, 1)) |
               BitBoard::Fill(Position(3, 1)) | BitBoard::Fill(Position(1, 1));
  auto moves =
      kBitTables
          .cannon_col_moves[Position(5).value()][board.GetColOccupancy(1)];

  auto expected_moves = BitBoard::Fill(Position(7, 0)) |
                        BitBoard::Fill(Position(4, 0)) |
                        BitBoard::Fill(Position(1, 0));
  return moves == expected_moves;
}

bool CheckRookRowTables() {
  auto board = BitBoard::Fill(Position(5, 1)) | BitBoard::Fill(Position(5, 5)) |
               BitBoard::Fill(Position(5, 7));
  auto moves =
      kBitTables
          .rook_row_moves[Position(1).value()][board.GetRowOccupancy(5)];
  auto expected_moves =
      BitBoard::Fill(Position(0, 0)) | BitBoard::Fill(Position(0, 2)) |
      BitBoard::Fill(Position(0, 3)) | BitBoard::Fill(Position(0, 4)) |
      BitBoard::Fill(Position(0, 5));
  return moves == expected_moves;
}

bool CheckRookColTables() {
  auto board = BitBoard::Fill(Position(5, 1)) | BitBoard::Fill(Position(6, 1)) |
               BitBoard::Fill(Position(7, 1)) | BitBoard::Fill(Position(9, 1)) |
               BitBoard::Fill(Position(3, 1)) | BitBoard::Fill(Position(1, 1));
  auto moves =
      kBitTables
          .cannon_col_moves[Position(5).value()][board.GetColOccupancy(1)];

  auto expected_moves = BitBoard::Fill(Position(3, 0)) |
                        BitBoard::Fill(Position(4, 0)) |
                        BitBoard::Fill(Position(6, 0));
  return moves == expected_moves;
}

int main() {
  bool success = true;
  success = success && TestPawnReverseTables();
  success = success && TestElephantOccupancy();
  success = success && TestHorseOccupancy();
  success = success && CheckHorseTables();
  success = success && CheckHorseReverseTables();
  success = success && TestRowOccupancy();
  success = success && CheckCannonRowTables();
  success = success && TestColOccupancy();
  success = success && CheckCannonColTables();
  success = success && CheckRookRowTables();
  cout << (success ? "Success." : "Failed.") << endl;
  return success ? 0 : 1;
}
