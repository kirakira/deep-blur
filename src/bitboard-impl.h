#ifdef BLUR_BITBOARD_H  // Requires including bitboard.h.

#include <utility>

namespace {
int GetBit(uint64 x, int pos) { return (x >> pos) & 1; }
}  // namespace

/* static */
constexpr HalfBitBoard HalfBitBoard::EmptyBoard() { return HalfBitBoard(0); }

/* static */
constexpr HalfBitBoard HalfBitBoard::Fill(Position pos) {
  return HalfBitBoard(static_cast<uint64>(1) << pos.value());
}

HalfBitBoard operator~(HalfBitBoard b) {
  return HalfBitBoard(((static_cast<uint64>(1) << (kNumPositions / 2)) - 1) &
                      ~b.value_);
}

HalfBitBoard operator&(HalfBitBoard b1, HalfBitBoard b2) {
  return HalfBitBoard(b1.value_ & b2.value_);
}

HalfBitBoard& HalfBitBoard::operator&=(HalfBitBoard b) {
  value_ &= b.value_;
  return *this;
}

constexpr HalfBitBoard operator|(HalfBitBoard b1, HalfBitBoard b2) {
  return HalfBitBoard(b1.value_ | b2.value_);
}

constexpr HalfBitBoard& HalfBitBoard::operator|=(HalfBitBoard b) {
  value_ |= b.value_;
  return *this;
}

HalfBitBoard operator^(HalfBitBoard b1, HalfBitBoard b2) {
  return HalfBitBoard(b1.value_ ^ b2.value_);
}

HalfBitBoard& HalfBitBoard::operator^=(HalfBitBoard b) {
  value_ ^= b.value_;
  return *this;
}

bool operator==(HalfBitBoard b1, HalfBitBoard b2) {
  return b1.value_ == b2.value_;
}

bool operator!=(HalfBitBoard b1, HalfBitBoard b2) {
  return !(b1.value_ == b2.value_);
}

std::ostream& operator<<(std::ostream& os, HalfBitBoard b) {
  for (int i = 4; i >= 0; --i) {
    for (int j = 0; j < kNumColumns; ++j) {
      os << (GetBit(b.value_, i * kNumColumns + j) ? "x" : ".");
    }
    os << std::endl;
  }
  return os;
}

/* static */
constexpr BitBoard BitBoard::EmptyBoard() {
  return BitBoard(HalfBitBoard::EmptyBoard(), HalfBitBoard::EmptyBoard());
}

/* static */
constexpr BitBoard BitBoard::Fill(Position pos) {
  return pos.value() < 45
             ? BitBoard(HalfBitBoard::Fill(pos), HalfBitBoard::EmptyBoard())
             : BitBoard(HalfBitBoard::EmptyBoard(),
                        HalfBitBoard::Fill(Position(pos.value() - 45)));
}

BitBoard operator~(BitBoard b) {
  return BitBoard(~b.halves_[0], ~b.halves_[1]);
}

constexpr BitBoard operator|(BitBoard b1, BitBoard b2) {
  return BitBoard(b1.halves_[0] | b2.halves_[0], b1.halves_[1] | b2.halves_[1]);
}

constexpr BitBoard& BitBoard::operator|=(BitBoard b) {
  halves_[0] |= b.halves_[0];
  halves_[1] |= b.halves_[1];
  return *this;
}

BitBoard operator&(BitBoard b1, BitBoard b2) {
  return BitBoard(b1.halves_[0] & b2.halves_[0], b1.halves_[1] & b2.halves_[1]);
}

BitBoard& BitBoard::operator&=(BitBoard b) {
  halves_[0] &= b.halves_[0];
  halves_[1] &= b.halves_[1];
  return *this;
}

BitBoard operator^(BitBoard b1, BitBoard b2) {
  return BitBoard(b1.halves_[0] ^ b2.halves_[0], b1.halves_[1] ^ b2.halves_[1]);
}

BitBoard& BitBoard::operator^=(BitBoard b) {
  halves_[0] ^= b.halves_[0];
  halves_[1] ^= b.halves_[1];
  return *this;
}

bool operator==(BitBoard b1, BitBoard b2) {
  return b1.halves_[0] == b2.halves_[0] && b1.halves_[1] == b2.halves_[1];
}

bool operator!=(BitBoard b1, BitBoard b2) { return !(b1 == b2); }

std::ostream& operator<<(std::ostream& os, BitBoard b) {
  os << b.halves_[1];
  for (int i = 0; i < kNumColumns; ++i) os << "-";
  os << std::endl;
  os << b.halves_[0];
  return os;
}

namespace {

//    ---
//    |1|
// ---------
// |2|   |0|
// ---------
//    |3|
//    ---
constexpr int kAdjacentOffsets[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

// --- ---
// |1| |0|
// -------
//   | |
// -------
// |2| |3|
// --- ---
constexpr int kDiagonalOffsets[4][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};

constexpr bool InBoard(int i, int j) { return Position::IsValidPosition(i, j); }

constexpr bool InRedHalf(int i, int j) { return InBoard(i, j) && i < 5; }

constexpr bool InBlackHalf(int i, int j) {
  return InBoard(i, j) && !InRedHalf(i, j);
}

constexpr bool InRedPalace(int i, int j) {
  return InBoard(i, j) && i <= 2 && j >= 3 && j <= 5;
}

constexpr bool InBlackPalace(int i, int j) {
  return InBoard(i, j) && i >= 7 && j >= 3 && j <= 5;
}

template <typename... Index>
constexpr std::array<std::pair<int, int>, sizeof...(Index)> MakeOffsets(
    const int offset_array[][2], Index... indices) {
  return {{std::pair<int, int>(offset_array[indices][0],
                               offset_array[indices][1])...}};
}

template <typename Predicate, size_t array_length>
constexpr BitBoard RelativePositions(
    int i, int j, Predicate predicate,
    const std::array<std::pair<int, int>, array_length>& offsets) {
  auto result = BitBoard::EmptyBoard();
  // Did not use range-based for-loop because stupid C++14 does not mark
  // std::array::begin() as constexpr.
  for (size_t index = 0; index < offsets.size(); ++index) {
    const int ni = i + offsets[index].first, nj = j + offsets[index].second;
    if (predicate(ni, nj)) {
      result |= BitBoard::Fill(Position(ni, nj));
    }
  }
  return result;
}

constexpr BitBoard RedPawnMovesAt(size_t index) {
  Position pos(index);
  const int i = pos.Row(), j = pos.Column();
  if (InRedHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 1));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 1, 2));
  }
}

constexpr BitBoard BlackPawnMovesAt(size_t index) {
  Position pos(index);
  const int i = pos.Row(), j = pos.Column();
  if (InBlackHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 3));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 2, 3));
  }
}

constexpr BitBoard AssistantMovesAt(size_t index) {
  Position pos(index);
  const int i = pos.Row(), j = pos.Column();
  if (InRedPalace(i, j)) {
    return RelativePositions(i, j, InRedPalace,
                             MakeOffsets(kDiagonalOffsets, 0, 1, 2, 3));
  } else if (InBlackPalace(i, j)) {
    return RelativePositions(i, j, InBlackPalace,
                             MakeOffsets(kDiagonalOffsets, 0, 1, 2, 3));
  } else {
    return BitBoard::EmptyBoard();
  }
}

}  // namespace

#endif  // BLUR_BITBOARD_H
