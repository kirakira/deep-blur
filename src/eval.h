#ifndef BLUR_EVAL_H
#define BLUR_EVAL_H

#include <memory>
#include <string>

#include "board-base.h"

namespace blur {

class Evaluator {
public:
  virtual ~Evaluator() {}

  // Possible evaluator names:
  //   "piece-position" (default)
  //   "piece-value".
  static std::unique_ptr<Evaluator>
  Make(const std::string &evaluator_name = "piece-position");

  // Return a copy of this evaluator that has the identical internal state.
  virtual std::unique_ptr<Evaluator> Clone() const = 0;

  // Update the evaluation in response to a move.
  virtual void OnMake(Move move, Piece moving_piee, Piece captured_piece) = 0;
  // Update the evaluation in response to the undo of a move.
  virtual void OnUnmake(Move move, Piece moving_piece,
                        Piece captured_piece) = 0;
  // Recalculate the evalution based on current the board position. board is
  // an array of 90 elements, indexed by the positions of each piece.
  virtual void SetBoard(const Piece board[]) = 0;

  // Get the evaluation of the current board position. Positive scores are
  // favorable to red.
  virtual Score CurrentScore() const = 0;
};

} // namespace blur

#endif // BLUR_EVAL_H
