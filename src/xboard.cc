#include <iostream>

#include "board.h"
#include "search.h"

using std::cout;
using std::endl;
using blur::Board;
using blur::Side;

int main() {
  Board board;
  const auto result = blur::Search(&board, Side::kRed, 7);
  cout << result.score << " " << result.best_move.ToString() << endl;
  blur::DebugPrintLogs();
  return 0;
}
