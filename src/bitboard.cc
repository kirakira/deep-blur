#include <vector>

#include "bitboard.h"

using std::vector;

namespace blur {

namespace {

constexpr int GetBit(uint64 x, int pos) { return (x >> pos) & 1; }

//    ---
//    |1|
// ---------
// |2|   |0|
// ---------
//    |3|
//    ---
constexpr int di[] = {0, 1, 0, -1};
constexpr int dj[] = {1, 0, -1, 0};

constexpr bool InRedHalf(Position pos) { return pos.Row() < 5; }

constexpr bool InBlackHalf(Position pos) { return !InRedHalf(pos); }

}  // namespace

/* static */
constexpr HalfBitBoard HalfBitBoard::EmptyBoard() noexcept {
  return HalfBitBoard(0);
}

/* static */
constexpr HalfBitBoard HalfBitBoard::Fill(Position pos) noexcept {
  return HalfBitBoard(static_cast<uint64>(1) << pos.value());
}

constexpr HalfBitBoard operator~(HalfBitBoard b) noexcept {
  return HalfBitBoard(((static_cast<uint64>(1) << (kNumPositions / 2)) - 1) &
                      ~b.value_);
}

constexpr HalfBitBoard operator&(HalfBitBoard b1, HalfBitBoard b2) noexcept {
  return HalfBitBoard(b1.value_ & b2.value_);
}

HalfBitBoard& HalfBitBoard::operator&=(HalfBitBoard b) noexcept {
  value_ &= b.value_;
  return *this;
}

constexpr HalfBitBoard operator|(HalfBitBoard b1, HalfBitBoard b2) noexcept {
  return HalfBitBoard(b1.value_ | b2.value_);
}

HalfBitBoard& HalfBitBoard::operator|=(HalfBitBoard b) noexcept {
  value_ |= b.value_;
  return *this;
}

constexpr HalfBitBoard operator^(HalfBitBoard b1, HalfBitBoard b2) noexcept {
  return HalfBitBoard(b1.value_ ^ b2.value_);
}

HalfBitBoard& HalfBitBoard::operator^=(HalfBitBoard b) noexcept {
  value_ ^= b.value_;
  return *this;
}

constexpr bool operator==(HalfBitBoard b1, HalfBitBoard b2) noexcept {
  return b1.value_ == b2.value_;
}

constexpr bool operator!=(HalfBitBoard b1, HalfBitBoard b2) noexcept {
  return !(b1.value_ == b2.value_);
}

std::ostream& operator<<(std::ostream& os, HalfBitBoard b) {
  for (int i = 4; i >= 0; ++i) {
    for (int j = 0; j < kNumColumns; ++j) {
      os << (GetBit(b.value_, i * kNumColumns + j) ? "x" : ".");
    }
    os << std::endl;
  }
  return os;
}

/* static */
constexpr BitBoard BitBoard::EmptyBoard() noexcept {
  return BitBoard(HalfBitBoard::EmptyBoard(), HalfBitBoard::EmptyBoard());
}

/* static */
constexpr BitBoard BitBoard::Fill(Position pos) noexcept {
  return pos.value() < 45
             ? BitBoard(HalfBitBoard::Fill(pos), HalfBitBoard::EmptyBoard())
             : BitBoard(HalfBitBoard::EmptyBoard(),
                        HalfBitBoard::Fill(Position(pos.value() - 45)));
}

constexpr BitBoard operator~(BitBoard b) noexcept {
  return BitBoard(~b.halves_[0], ~b.halves_[1]);
}

constexpr BitBoard operator|(BitBoard b1, BitBoard b2) noexcept {
  return BitBoard(b1.halves_[0] | b2.halves_[0], b1.halves_[1] | b2.halves_[1]);
}

BitBoard& BitBoard::operator|=(BitBoard b) noexcept {
  halves_[0] |= b.halves_[0];
  halves_[1] |= b.halves_[1];
  return *this;
}

constexpr BitBoard operator&(BitBoard b1, BitBoard b2) noexcept {
  return BitBoard(b1.halves_[0] & b2.halves_[0], b1.halves_[1] & b2.halves_[1]);
}

BitBoard& BitBoard::operator&=(BitBoard b) noexcept {
  halves_[0] &= b.halves_[0];
  halves_[1] &= b.halves_[1];
  return *this;
}

constexpr BitBoard operator^(BitBoard b1, BitBoard b2) noexcept {
  return BitBoard(b1.halves_[0] ^ b2.halves_[0], b1.halves_[1] ^ b2.halves_[1]);
}

BitBoard& BitBoard::operator^=(BitBoard b) noexcept {
  halves_[0] ^= b.halves_[0];
  halves_[1] ^= b.halves_[1];
  return *this;
}

constexpr bool operator==(BitBoard b1, BitBoard b2) noexcept {
  return b1.halves_[0] == b2.halves_[0] && b1.halves_[1] == b2.halves_[1];
}

constexpr bool operator!=(BitBoard b1, BitBoard b2) noexcept {
  return !(b1 == b2);
}

std::ostream& operator<<(std::ostream& os, BitBoard b) {
  os << b.halves_[1];
  for (int i = 0; i < kNumColumns; ++i) os << "-";
  os << std::endl;
  os << b.halves_[0];
  return os;
}

constexpr BitTables::BitTables() noexcept {
  FillRedPawnMoves();
  FillBlackPawnMoves();
}

void BitTables::FillRedPawnMoves() noexcept {
  for (int i = 3; i < kNumRows; ++i) {
    for (int j = 0; j < kNumColumns; ++j) {
      Position pos(i, j);
      red_pawn_moves[pos.value()] = BitBoard::EmptyBoard();

      vector<int> enabled_directions;
      if (InRedHalf(pos)) {
        enabled_directions = {1};
      } else {
        enabled_directions = {0, 1, 2};
      }

      for (int r : enabled_directions) {
        int ni = i + di[r], nj = j + dj[r];
        if (Position::IsValidPosition(ni, nj)) {
          Position next(ni, nj);
          red_pawn_moves[pos.value()] |= BitBoard::Fill(next);
        }
      }
    }
  }
}

void BitTables::FillBlackPawnMoves() noexcept {
  for (int i = 0; i <= 6; ++i) {
    for (int j = 0; j < kNumColumns; ++j) {
      Position pos(i, j);
      black_pawn_moves[pos.value()] = BitBoard::EmptyBoard();

      vector<int> enabled_directions;
      if (InBlackHalf(pos)) {
        enabled_directions = {3};
      } else {
        enabled_directions = {0, 2, 3};
      }

      for (int r : enabled_directions) {
        int ni = i + di[r], nj = j + dj[r];
        if (Position::IsValidPosition(ni, nj)) {
          Position next(ni, nj);
          black_pawn_moves[pos.value()] |= BitBoard::Fill(next);
        }
      }
    }
  }
}

}  // namespace blur
