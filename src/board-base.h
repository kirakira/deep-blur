#ifndef BLUR_BOARD_BASE_H
#define BLUR_BOARD_BASE_H

#include <string>

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
  // Construct an uninitialized Position.
  Position() : Position(0) {}
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
  bool IsValid() const { return value_ >= 0 && value_ < kNumPositions; }
  constexpr int Row() const { return value_ / kNumColumns; }
  constexpr int Column() const { return value_ % kNumColumns; }
  constexpr bool InRedHalf() const { return value_ < (kNumPositions / 2); }
  std::string ToString() const;

  Position(const Position&) = default;
  Position& operator=(const Position&) = default;

  friend bool operator==(Position x, Position y) {
    return x.value_ == y.value_;
  }

  friend bool operator!=(Position x, Position y) { return !(x == y); }

 private:
  int value_;
};

// Value semantics.
class Move {
 public:
  // An unintialized move.
  Move() = default;
  Move(Position from, Position to) : from_(from), to_(to) {}
  explicit Move(const std::string& str);
  // value must be the return value of value().
  explicit Move(int value) : from_(value >> 7), to_(value & ((1 << 7) - 1)) {}

  // Return value ranges in [0, 2^14).
  int value() const { return (from_.value() << 7) | (to_.value()); }
  bool IsValid() const {
    return from_.IsValid() && to_.IsValid() && from_ != to_;
  }

  Position from() const { return from_; }
  Position to() const { return to_; }
  std::string ToString() const;

  Move(const Move&) = default;
  Move& operator=(const Move&) = default;

  friend bool operator==(Move a, Move b) {
    return a.from_ == b.from_ && a.to_ == b.to_;
  }

  friend bool operator!=(Move a, Move b) { return !(a == b); }

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

// Helper function to negate side.
constexpr Side OtherSide(Side side) {
  return side == Side::kRed ? Side::kBlack : Side::kRed;
}

// A piece is a pair of (Side, PieceType). Value semantics.
class Piece {
 public:
  // An uninitialized piece.
  Piece() = default;
  constexpr Piece(Side side, PieceType piece_type)
      : value_(static_cast<int>(piece_type) | (static_cast<int>(side) << 3)) {}
  // A special piece denotes emtpy. Compares to false with any real piece. Don't
  // call side() or type() on it.
  constexpr static Piece EmptyPiece() { return Piece(0); }

  constexpr Side side() const {
    return static_cast<Side>(value_ >> 3);
  }

  constexpr PieceType type() const {
    return static_cast<PieceType>(value_ & 7);
  }

  // value ranges in [0, 16).
  constexpr int value() const {
    return value_;
  }

  // Convert the piece to its single-letter representation.
  char ToLetter() const {
    static const char letter_map[] = " KPAEHCR kpaehcr";
    return letter_map[value_];
  }

  Piece(const Piece&) = default;
  Piece& operator=(const Piece&) = default;

  friend bool operator==(Piece x, Piece y) { return x.value_ == y.value_; }
  friend bool operator!=(Piece x, Piece y) { return !(x == y); }

 private:
  constexpr Piece(int value) : value_(value) {}

  int value_;
};

class MoveList {
 public:
  template <typename... Args>
  inline void Add(Args&&... args) {
    // Placement new saves us cost to zero memory upfront.
    new (PointerOf(size_++)) Move(std::forward<Args>(args)...);
  }
  ~MoveList() {
    // We need to manually call the destructors since they are placement-newed
    // (even though they are trivial destructors).
    for (int i = 0; i < size_; ++i) {
      PointerOf(i)->~Move();
    }
  }

  int size() const { return size_; }

  inline Move* begin() { return PointerOf(0); }
  inline Move* end() { return PointerOf(size_); }
  inline const Move* begin() const { return PointerOf(0); }
  inline const Move* end() const { return PointerOf(size_); }

 private:
  Move* PointerOf(int i) {
    return reinterpret_cast<Move*>(buffer_ + i * sizeof(Move));
  }
  const Move* PointerOf(int i) const {
    return reinterpret_cast<const Move*>(buffer_ + i * sizeof(Move));
  }

  static constexpr int kArraySize = 120;
  // We use placement new to avoid the array from being zero'd.
  char buffer_[sizeof(Move) * kArraySize];
  int size_ = 0;
};

enum class MoveType {
  kRegular,
  kRepetition,
  kPerpetualAttacker,
  kPerpetualAttackee
};

enum Score : int {
  kDrawScore = 0,
  kPawnScore = 100,
  kAssistantScore = 150,
  kElephantScore = 150,
  kHorseScore = 400,
  kCannonScore = 400,
  kRookScore = 900,
  kMateScore = 10000
};

inline Score& operator+=(Score& x, Score y) {
  return x = static_cast<Score>(x + y);
}

inline Score& operator*=(Score& x, int y) {
  return x = static_cast<Score>(x * y);
}

inline Score operator*(Score x, int y) {
  return static_cast<Score>(static_cast<int>(x) * y);
}

inline Score operator+(Score x, Score y) {
  return static_cast<Score>(static_cast<int>(x) + static_cast<int>(y));
}

inline Score operator-(Score x, Score y) {
  return static_cast<Score>(static_cast<int>(x) - static_cast<int>(y));
}

inline Score operator-(Score x) {
  return static_cast<Score>(-static_cast<int>(x));
}

}  // namespace blur

#endif  // BLUR_BOARD_BASE_H
