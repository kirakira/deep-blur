#ifndef BLUR_EVAL_H
#define BLUR_EVAL_H

#include "board-base.h"

namespace blur {

class Evaluator {
 public:
  // Update the evaluation in response to a move.
  void OnMake(Move move, Piece moving_piee, Piece captured_piece);
  // Update the evaluation in response to the undo of a move.
  void OnUnmake(Move move, Piece moving_piece, Piece captured_piece);
  // Recalculate the evalution based on current the board position. board is
  // an array of 90 elements, indexed by the positions of each piece.
  void SetBoard(const Piece board[]);

  // Get the evaluation of the current board position. Positive scores are
  // favorable to red.
  Score CurrentScore() const;

 private:
  Score score_ = kDrawScore;
};

}  // namespace blur

#endif  // BLUR_EVAL_H
