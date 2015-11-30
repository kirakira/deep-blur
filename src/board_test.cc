#include <iostream>

#include "board.h"

using namespace blur;
using namespace std;

void PrintPawnTables() {
  for (int i = 0; i < kNumPositions; ++i) {
    cout << i << ": " << kBitTables.red_pawn_moves[i] << endl;
  }
}

int main() {
  PrintPawnTables();
  return 0;
}
