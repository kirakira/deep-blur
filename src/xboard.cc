#include <iostream>

#include "board.h"
#include "search.h"

using std::cout;
using std::endl;
using blur::Board;
using blur::Side;
using blur::TranspositionTable;

int main() {
  Board board;
  TranspositionTable tt;
  const auto result = blur::Search(&board, &tt, Side::kRed, 9);
  cout << result.score << " " << result.best_move.ToString() << endl;
  blur::DebugPrintLogs();
  return 0;
}
