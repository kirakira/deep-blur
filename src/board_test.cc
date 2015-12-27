#include <iostream>

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

bool CheckElephantTables() {
  auto board = BitBoard::Fill(Position(67 + 10)) | BitBoard::Fill(Position(67 - 8));
  uint64 occ = board.GetElephantOccupancy(Position(67));
  auto expected = BitBoard::Fill(Position(67 + 16)) | BitBoard::Fill(Position(67 - 20));
  return expected == BitTables::elephant_moves[67][occ];
}

int main() {
  PrintElephantTables();
  bool success = true;
  success = success && CheckElephantTables();
  return success ? 0 : 1;
}
