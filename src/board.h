#ifndef BLUR_BOARD_H
#define BLUR_BOARD_H

#include <string>
#include <vector>

#include "bitboard.h"
#include "board-base.h"
#include "common.h"

namespace blur {

class Board {
 public:
  // Default constructor initializes the board to a start position.
  Board();

  // Generate all pseudo-legal moves for the specified side. This includes
  // all legal moves plus suicides and perpectual checks and/or attacks.
  std::vector<Move> GenerateMoves(Side side) const;

  void Make(Move m);
  void Unmake();

  // Returns false if this is a bad board position.
  bool SetBoard(const std::string& fen);
  // Return the fen string of the current position.
  std::string ToString() const;
  // Print the current board.
  void DebugPrint() const;

 private:
  Piece board_[kNumRows][kNumColumns];
  // Indexed by piece.value().
  BitBoard piece_bitboards_[16];
};

}  // namespace blur

#endif  // BLUR_BOARD_H
