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

constexpr int kNumPositions = 90;
constexpr int kNumRows = 10;
constexpr int kNumColumns = 9;

// Value semantics.
class Position final {
 public:
  constexpr Position(int row, int col) noexcept
      : Position(row* kNumColumns + col) {}
  // For example, "a0" -> 0; "d8" -> 75.
  constexpr explicit Position(const std::string& str) noexcept;
  // pos values from 0 to 89 inclusive.
  constexpr explicit Position(int pos) noexcept : value_(pos) {}

  static constexpr bool IsValidPosition(int row, int col) noexcept {
    return row >= 0 && row < kNumRows && col >= 0 && col < kNumColumns;
  }

  static constexpr bool IsValidPosition(const std::string& str) noexcept;

  // Ranges from 0 to 89 inclusive.
  constexpr int value() const noexcept { return value_; }
  constexpr int Row() const noexcept { return value_ / kNumColumns; }
  constexpr int Column() const noexcept { return value_ % kNumColumns; }
  std::string ToString() const;

  Position(const Position&) = default;
  Position& operator=(const Position&) = default;

 private:
  int value_;
};

// Value semantics.
class Move final {
 public:
  explicit Move(Position from, Position to) noexcept;
  constexpr explicit Move(const std::string& str) noexcept;

  // Returned int uses its lower 14 bits.
  constexpr int value() const noexcept;

  constexpr Position from() const noexcept { return from_; }
  constexpr Position to() const noexcept { return to_; }
  std::string ToString() const;

  constexpr Move(const Move&) = default;
  Move& operator=(const Move&) = default;

 private:
  Position from_;
  Position to_;
};

}  // namespace blur

#endif  // BLUR_BOARD_COMMON_H
