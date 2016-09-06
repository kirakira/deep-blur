#ifndef BLUR_SEARCH_H
#define BLUR_SEARCH_H

#include "board.h"
#include "transposition.h"

namespace blur {

struct SearchResult {
  // Score is relative to side.
  Score score;
  // Will not be populated if score == -Evaluation::kMateSocre.
  Move best_move;
};

// Return value may not have a valid best_move if
// * depth is 0, or
// * score is -kMateScore.
SearchResult Search(Board* board, TranspositionTable* tt, Side side, int depth);

// This is no-op when macro NDEBUG is defined.
void DebugPrintLogs();

}  // namespace blur

#endif  // BLUR_SEARCH_H
