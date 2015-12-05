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

HalfBitBoard& HalfBitBoard::operator|=(HalfBitBoard b) {
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

BitBoard& BitBoard::operator|=(BitBoard b) {
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
constexpr int di[] = {0, 1, 0, -1};
constexpr int dj[] = {1, 0, -1, 0};

// --- ---
// |1| |0|
// -------
//   | |
// -------
// |2| |3|
// --- ---
constexpr int ddi[] = {1, 1, -1, -1};
constexpr int ddj[] = {1, -1, -1, 1};

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

template <typename Predicate, Predicate predicate>
constexpr BitBoard CombinePositionIf(BitBoard current,
                                     const std::pair<int, int>& pos) {
  if (predicate(pos.first, pos.second)) {
    return current | BitBoard::Fill(Position(pos.first, pos.second));
  } else {
    return current;
  }
}

template <typename Predicate, Predicate predicate, size_t... directions>
constexpr BitBoard AdjacentPositions(int i, int j) {
  return Aggregate(CombinePositionIf<Predicate, predicate>,
                   BitBoard::EmptyBoard(),
                   std::make_pair(i + di[directions], j + dj[directions])...);
}

template <typename Predicate>
constexpr BitBoard RelativePositions(
    int i, int j, std::initializer_list<std::pair<int, int>> offsets,
    Predicate predicate) {
  auto result = BitBoard::EmptyBoard();
  for (auto offset : offsets) {
    const int ni = i + offset.first, nj = j + offset.second;
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
    return AdjacentPositions<decltype(InBoard), InBoard, 1>(i, j);
  } else {
    return AdjacentPositions<decltype(InBoard), InBoard, 0, 1, 2>(i, j);
  }
}

constexpr BitBoard BlackPawnMovesAt(size_t index) {
  Position pos(index);
  const int i = pos.Row(), j = pos.Column();
  if (InBlackHalf(i, j)) {
    return AdjacentPositions<decltype(InBoard), InBoard, 3>(i, j);
  } else {
    return AdjacentPositions<decltype(InBoard), InBoard, 0, 2, 3>(i, j);
  }
}

constexpr BitBoard AssistantMovesAt(size_t index) {
  Position pos(index);
  const int i = pos.Row(), j = pos.Column();
  if (InRedPalace(i, j)) {
    return BitBoard::EmptyBoard();
  } else if (InBlackPalace(i, j)) {
    return BitBoard::EmptyBoard();
  } else {
    return BitBoard::EmptyBoard();
  }
}

}  // namespace

#endif  // BLUR_BITBOARD_H
