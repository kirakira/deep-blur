#ifndef BLUR_SEARCH_H
#define BLUR_SEARCH_H

#include "board.h"
#include "transposition.h"

namespace blur {

struct SearchResult {
  // Score is relative to side.
  Score score;
  // Will not be populated if score == -kMateSocre.
  Move best_move;
};

struct SearchOptions {
  bool enable_quiescence = true;

  static const SearchOptions& Defaults();
};

// Return value may not have a valid best_move if
// * depth is 0, or
// * score is -kMateScore.
SearchResult Search(Board* board, TranspositionTable* tt, Side side, int depth,
                    const SearchOptions& options = SearchOptions::Defaults());

// This is no-op when macro NDEBUG is defined.
void DebugPrintLogs();

}  // namespace blur

#endif  // BLUR_SEARCH_H
