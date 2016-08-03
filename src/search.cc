#include "search.h"

namespace blur {

// A class that unmakes the move in destructor.
class CheckedMoveMaker {
 public:
  CheckedMoveMaker(Board& board, Side side, Move move) : board_(board) {
    move_made_ = board.CheckedMake(side, move);
  }

  ~CheckedMoveMaker() {
    if (move_made_) board_.Unmake();
  }

  bool MoveMade() const { return move_made_; }

 private:
  Board& board_;
  bool move_made_;
};

// Only scores within (alpha, beta) are exact. Scores <= alpha are upper bounds;
// scores >= beta are lower bounds.
SearchResult Search(Board& board, Side side, int depth, Score alpha, Score beta) {
  SearchResult result;

  if (depth == 0) {
    Score score = board.Evaluation();
    result.score = (side == Side::kRed ? score : -score);
    return result;
  }

  result.score = -kMateScore;
  for (auto move : board.GenerateMoves(side)) {
    CheckedMoveMaker move_maker(board, side, move);
    if (!move_maker.MoveMade()) continue;

    const Score current_alpha = std::max(result.score, alpha);
    const auto child_result =
        Search(board, OtherSide(side), depth - 1, -beta, -current_alpha);
    const Score current_score = -child_result.score;

    if (current_score > result.score) {
      result.score = current_score;
      result.best_move = move;
    }

    if (result.score >= beta) break;
  }

  return result;
}

SearchResult Search(Board& board, Side side, int depth) {
  return Search(board, side, depth, -kMateScore, kMateScore);
}

}  // namespace blur
