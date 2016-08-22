#ifndef BLUR_BOARD_H
#define BLUR_BOARD_H

#include <string>
#include <utility>
#include <vector>

#include "bitboard.h"
#include "board-base.h"
#include "common.h"
#include "eval.h"

namespace blur {

class Board {
 public:
  // Initialize the board to the start position.
  Board();

  // Generate all pseudo-legal moves for the specified side. This includes
  // all legal moves plus suicides and perpectual checks and/or attacks.
  MoveList GenerateMoves(Side side) const;
  // Generate all pseudo-legal captures for the specified side.
  MoveList GenerateCaptures(Side side) const;
  // Is the position being attacked (by pieces of the other side). Returns
  // true/false and, in the case of true, position of the attacking piece with
  // least material value. Material values are ordered in this way:
  // PAWN < ASSISTANT = ELEPHANT < CANNON = HORSE < ROOK < KING.
  // Requires:
  //    pos must be occupied by a piece.
  std::pair<bool, Position> IsAttacked(Position pos) const;
  // Is side being checked?
  // Requires:
  //    King of side must exist.
  bool InCheck(Side side) const;

  MoveType Make(Move m);
  // This only accepts legal moves (no suicides). King captures are ok.
  std::pair<bool, MoveType> CheckedMake(Side side, Move m);
  void Unmake();
  bool CheckedUnmake();

  // Calculate repetition from current position.
  void ResetRepetitionHistory();

  uint64 HashCode(Side side) const;

  // Returns false if this is a bad board position.
  bool SetBoard(const std::string& fen);
  // Return the fen string of the current position.
  std::string ToString() const;
  // Print the current board.
  void DebugPrint() const;

  Score Evaluation() const { return eval_.CurrentScore(); }

 private:
  struct HistoryMove {
    // Unsided hash.
    uint64 hash_before_move;
    Move move;
    Piece capture;
  };

  void MakeWithoutRepetitionDetection(Move m);
  // TODO: Cache the following?
  BitBoard AllPiecesMask() const;
  BitBoard SidePiecesMask(Side side) const;
  // Generate all pseudo-legal moves and self-captures for side, and call f on
  // every move generated. visitor should have the following signature:
  // void f(Position from, BitBoard to).
  template <typename Function>
  void VisitMoves(Side side, Function visitor) const;
  // Generate moves for side whose destinations are restricted to a subset of
  // positions of board specified by allowed.
  MoveList GenerateMovesWithAllowedMask(Side side, BitBoard allowed) const;
  // Return a bitboard of positions controlled by side.
  BitBoard ControlledPositions(Side side) const;
  MoveType GetRepetitionType(int first_move_index);
  bool IsChasing(Position pos) const;

  Piece board_[kNumPositions];
  // Indexed by piece.value().
  BitBoard piece_bitboards_[16];
  std::vector<HistoryMove> history_;
  std::vector<int> irreversible_moves_;
  int repetition_start_ = 0;
  uint64 hash_;
  Evaluator eval_;
};

}  // namespace blur

#endif  // BLUR_BOARD_H
