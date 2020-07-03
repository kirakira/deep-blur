#ifndef BLUR_SEARCH_H
#define BLUR_SEARCH_H

#include <chrono>

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
  bool use_tt = true;
  bool enable_quiescence = true;
  bool use_tt_in_quiescence = false;
  std::chrono::milliseconds time_limit = std::chrono::seconds(5);

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
