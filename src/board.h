#ifndef BLUR_BOARD_H
#define BLUR_BOARD_H

#include <string>
#include <vector>

#include "bitboard.h"
#include "common.h"

namespace blur {

// Numbering board positions
//
//      a  b  c  d  e  f  g  h  i
//    ----------------------------
// 9  |81|82|83|84|85|86|87|88|89|
// 8  |72|73|74|75|76|77|78|79|80|
// 7  |63|        ....        |71|   UPPER HALF (BLACK)
// 6  |54|        ....        |62|
// 5  |45|46|47|48|49|50|51|52|53|
//    ----------------------------
//    |           RIVER          |
//    ----------------------------
// 4  |36|37|38|39|40|41|42|43|44|
// 3  |27|        ....        |35|
// 2  |18|        ....        |26|   LOWER HALF (RED)
// 1  | 9|10|11|12|13|14|15|16|17|
// 0  | 0| 1| 2| 3| 4| 5| 6| 7| 8|
//    ----------------------------

const int kNumPositions = 90;
const int kNumRows = 10;
const int kNumColumns = 9;

// Value semantics.
class Position {
 public:
  constexpr Position(int row, int col) : Position(row * kNumColumns + col) {}
  // For example, "a0" -> 0; "d8" -> 75.
  explicit Position(const std::string& str);
  // pos values from 0 to 89 inclusive.
  constexpr explicit Position(int pos) : value_(pos) {}

  constexpr static bool IsValidPosition(int row, int col) {
    return row >= 0 && row < kNumRows && col >= 0 && col < kNumColumns;
  }

  static bool IsValidPosition(const std::string& str);

  // Ranges from 0 to 89 inclusive.
  constexpr int value() const { return value_; }
  constexpr int Row() const { return value_ / kNumColumns; }
  constexpr int Column() const { return value_ % kNumColumns; }
  constexpr bool InRedHalf() const { return value_ < (kNumPositions / 2); }
  std::string ToString() const;

  Position(const Position&) = default;
  Position& operator=(const Position&) = default;

 private:
  int value_;
};

// Value semantics.
class Move {
 public:
  // An unintialized move.
  Move() = default;
  explicit Move(Position from, Position to);
  explicit Move(const std::string& str);

  // Return value ranges in [0, 2^14).
  int value() const;

  Position from() const { return from_; }
  Position to() const { return to_; }
  std::string ToString() const;

  Move(const Move&) = default;
  Move& operator=(const Move&) = default;

 private:
  Position from_;
  Position to_;
};

enum class PieceType {
  kKing = 1,
  kPawn,
  kAssistant,
  kElephant,
  kHorse,
  kCannon,
  kRook
};

enum class Side { kRed = 0, kBlack = 1 };

// A piece is a pair of (Side, PieceType). Value semantics.
class Piece {
 public:
  constexpr Piece(Side side, PieceType piece_type)
      : value_(piece_type | (side << 3)) {}
  // A special piece denotes emtpy. Compares to false with any real piece. Don't
  // call side() or type() on it.
  constexpr static Piece EmptyPiece() { return Piece(0); }

  inline Side side() const {
    return value_ >> 3;
  }

  inline PieceType type() const {
    return value_ & 7;
  }

  // value ranges in [0, 16).
  inline int value() const {
    return value_;
  }

  // Convert the piece to its single-letter representation.
  char ToLetter() const;

  Piece(const Piece&) = default;
  Piece& operator=(const Piece&) = default;

  friend bool operator==(Piece x, Piece y) { return x.value_ == y.value_; }
  friend bool operator!=(Piece x, Piece y) { return !(x == y); }

 private:
  constexpr Piece(int value) : value_(value) {}

  int value_;
};


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
  bool SetBoard(const string& fen);
  // Return the fen string representation of current position.
  std::string ToString() const;

 private:
  Piece board_[kNumRows][kNumColumns];
  BitBoard piece_bitboards_[16];
};

}  // namespace blur

#endif  // BLUR_BOARD_H
