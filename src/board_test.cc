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

void PrintAssistantTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << endl << BitTables::assistant_moves[i] << endl;
  }
}

int main() {
  PrintAssistantTables();
  return 0;
}
