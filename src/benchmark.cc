#include <iostream>

#include "board.h"
#include "search.h"

using std::cout;
using std::endl;
using blur::Board;
using blur::Side;
using blur::TranspositionTable;

namespace {

void go(Board* board, TranspositionTable* tt, Side* side, int depth) {
  auto result = blur::Search(board, tt, *side, depth);
  blur::DebugPrintLogs();

  if (result.score != -blur::kMateScore) {
    CHECK(board->CheckedMake(*side, result.best_move).first);
    cout << "move " << result.best_move.ToString() << endl;
    *side = blur::OtherSide(*side);
  } else {
    if (*side == Side::kRed) {
      cout << "0-1 {White resigns}" << endl;
    } else {
      cout << "1-0 {Black resigns}" << endl;
    }
  }
}

}  // namespace

int main() {
  cout.setf(std::ios_base::unitbuf);

  Board board;
  TranspositionTable tt;
  Side side = Side::kRed;
  go(&board, &tt, &side, /*depth=*/10);

  return 0;
}
