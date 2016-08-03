#ifndef BLUR_SEARCH_H
#define BLUR_SEARCH_H

#include "board.h"

namespace blur {

struct SearchResult {
  // Score is relative to side.
  Score score;
  // Will not be populated if score == -Evaluation::kMateSocre.
  Move best_move;
};

SearchResult Search(Board& board, Side side, int depth);

}  // namespace blur

#endif  // BLUR_SEARCH_H
