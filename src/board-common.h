#ifndef BLUR_BOARD_COMMON_H
#define BLUR_BOARD_COMMON_H

#include <string>

#include "common.h"

namespace blur {

// Numbering board positions
//
//      a  b  c  d  e  f  g  h  i
//    ----------------------------
// 9  |81|82|83|84|85|86|87|88|89|
// 8  |72|73|74|75|76|77|78|79|80|
// 7  |63|        ....        |71|   UPPER HALF
// 6  |54|        ....        |62|
// 5  |45|46|47|48|49|50|51|52|53|
//    ----------------------------
//    |           RIVER          |
//    ----------------------------
// 4  |36|37|38|39|40|41|42|43|44|
// 3  |27|        ....        |35|
// 2  |18|        ....        |26|   LOWER HALF
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
  // An unintialzied move.
  Move() = default;
  explicit Move(Position from, Position to);
  explicit Move(const std::string& str);

  // Returned int uses its lower 14 bits.
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

}  // namespace blur

#endif  // BLUR_BOARD_COMMON_H
