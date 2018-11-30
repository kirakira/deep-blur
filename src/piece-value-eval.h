#ifndef BLUR_PIECE_VALUE_EVAL_H
#define BLUR_PIECE_VALUE_EVAL_H

#include "eval.h"

namespace blur {

class PieceValueEvaluator : public Evaluator {
public:
  PieceValueEvaluator() = default;
  PieceValueEvaluator(const PieceValueEvaluator &) = default;

  // Return a copy of this evaluator that has the identical internal state.
  std::unique_ptr<Evaluator> Clone() const override;

  void OnMake(Move move, Piece moving_piee, Piece captured_piece) override;
  // Update the evaluation in response to the undo of a move.
  void OnUnmake(Move move, Piece moving_piece, Piece captured_piece) override;
  // Recalculate the evalution based on current the board position. board is
  // an array of 90 elements, indexed by the positions of each piece.
  void SetBoard(const Piece board[]) override;

  // Get the evaluation of the current board position. Positive scores are
  // favorable to red.
  Score CurrentScore() const override;

private:
  Score score_ = kDrawScore;
};

} // namespace blur

#endif // BLUR_PIECE_VALUE_EVAL_H
