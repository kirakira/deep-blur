#ifndef BLUR_BITBOARD_H
#define BLUR_BITBOARD_H

#include <array>
#include <iostream>
#include <utility>

#include "board-base.h"
#include "common.h"

namespace blur {

// Value semantics.
class HalfBitBoard {
 public:
  // An uninitialized HalfBitBoard.
  HalfBitBoard() = default;
  constexpr static HalfBitBoard EmptyBoard();
  // Returns a HalfBitBoard that has only the specified position set.
  // Requires: position.value() < 45.
  constexpr static HalfBitBoard Fill(Position position);

  // Returns lower 4 bits. From least significant: BL, TL, BR, TR.
  inline uint64 GetElephantOccupancy(Position pos) const;
  // Returns lower 4 bits. From least significant: L, B, R, T.
  // Requires pos range: [-9, 53].
  inline uint64 GetHorseOccupancy(int pos) const;
  inline uint64 GetRowOccupancy(int row) const;
  inline uint64 GetColOccupancy(int col) const;

  // Visit all positions set in this HalfBitBoard.
  template <typename Function>
  inline void VisitPositions(Function f) const {
    uint64 current = value_;
    while (current != 0) {
      f(Position(lsb(current)));
      current &= (current - 1);
    }
  }

  // Return any of the positions set. Requires non-empty board.
  inline Position AnyPosition() const {
    DCHECK(value_ != 0);
    return Position(lsb(value_));
  }

  inline HalfBitBoard(const HalfBitBoard&) = default;
  inline HalfBitBoard& operator=(const HalfBitBoard&) = default;

  // Extract a bit.
  // Requires: 0 <= pos < 45.
  inline bool operator[](int pos) const { return GetBit(value_, pos) == 1; }

  friend inline HalfBitBoard operator~(HalfBitBoard b);
  friend inline HalfBitBoard operator&(HalfBitBoard b1, HalfBitBoard b2);
  friend constexpr HalfBitBoard operator|(HalfBitBoard b1, HalfBitBoard b2);
  friend inline HalfBitBoard operator^(HalfBitBoard b1, HalfBitBoard b2);
  inline HalfBitBoard& operator&=(HalfBitBoard b);
  constexpr HalfBitBoard& operator|=(HalfBitBoard b);
  inline HalfBitBoard& operator^=(HalfBitBoard b);
  friend inline bool operator==(HalfBitBoard b1, HalfBitBoard b2);
  friend inline bool operator!=(HalfBitBoard b1, HalfBitBoard b2);

  // For debug output only.
  friend inline std::ostream& operator<<(std::ostream& os, HalfBitBoard b);

 private:
  constexpr explicit HalfBitBoard(uint64 value) : value_(value) {}
  inline uint64 GatherBits(uint64 relevant_mask, uint64 magic, int shift,
                           uint64 final_mask) const;

  uint64 value_;
};

// Value semantics.
class BitBoard {
 public:
  // An uninitialized BitBoard.
  BitBoard() = default;
  constexpr static BitBoard EmptyBoard();
  constexpr static BitBoard Fill(Position position);

  inline void Make(Move move);
  inline void Unmake(Move move);

  // Returns lower 4 bits. From least significant: BL, TL, BR, TR.
  inline uint64 GetElephantOccupancy(Position pos) const;
  // Returns lower 4 bits. From least significant: L, B, R, T.
  inline uint64 GetHorseOccupancy(Position pos) const;
  inline uint64 GetRowOccupancy(int row) const;
  inline uint64 GetColOccupancy(int col) const;

  // Visit all positions set in this BitBoard.
  template <typename Function>
  void VisitPositions(Function f) const {
    halves_[0].VisitPositions(f);
    halves_[1].VisitPositions(
        [f](Position pos) { f(Position(pos.value() + 45)); });
  }

  // Return any of the set positions. Requires non-empty board.
  inline Position AnyPosition() const {
    return halves_[0] == HalfBitBoard::EmptyBoard()
               ? Position(halves_[1].AnyPosition().value() + 45)
               : halves_[0].AnyPosition();
  }

  HalfBitBoard lower() const { return halves_[0]; }
  HalfBitBoard upper() const { return halves_[1]; }
  // Is the specified position set?
  inline bool operator[](Position pos) const;

  inline BitBoard(const BitBoard&) = default;
  inline BitBoard& operator=(const BitBoard&) = default;

  friend inline BitBoard operator~(BitBoard b);
  friend inline BitBoard operator&(BitBoard b1, BitBoard b2);
  friend constexpr BitBoard operator|(BitBoard b1, BitBoard b2);
  friend inline BitBoard operator^(BitBoard b1, BitBoard b2);
  inline BitBoard& operator&=(BitBoard b);
  constexpr BitBoard& operator|=(BitBoard b);
  inline BitBoard& operator^=(BitBoard b);
  friend inline bool operator==(BitBoard b1, BitBoard b2);
  friend inline bool operator!=(BitBoard b1, BitBoard b2);

  // For debug output only.
  friend inline std::ostream& operator<<(std::ostream& os, BitBoard b);

 private:
  constexpr BitBoard(HalfBitBoard lower, HalfBitBoard upper)
      : halves_{lower, upper} {}

  HalfBitBoard halves_[2];
};

/* static */
constexpr HalfBitBoard HalfBitBoard::EmptyBoard() { return HalfBitBoard(0); }

/* static */
constexpr HalfBitBoard HalfBitBoard::Fill(Position pos) {
  return HalfBitBoard(static_cast<uint64>(1) << pos.value());
}

uint64 HalfBitBoard::GatherBits(uint64 relevant_mask, uint64 magic, int shift,
                                uint64 final_mask) const {
  return (((value_ & relevant_mask) * magic) >> shift) & final_mask;
}

namespace impl {

constexpr uint64 GenerateElephantRelevantBits(int pos) {
  const Position p(pos);
  const int row = p.Row(), col = p.Column();
  uint64 relevant_mask = 0;
  if (row != 0 && col != 0) relevant_mask |= FillBits(pos - 10);
  if (row != 0 && col != 8) relevant_mask |= FillBits(pos - 8);
  if (row != 4 && col != 0) relevant_mask |= FillBits(pos + 8);
  if (row != 4 && col != 8) relevant_mask |= FillBits(pos + 10);
  return relevant_mask;
}

constexpr uint64 GenerateHorseRelevantBits(int pos_plus_9) {
  const int b = pos_plus_9 - 9;
  uint64 relevant_mask = 0;
  if (b >= 0 && b % 9 != 0) relevant_mask |= FillBits(b - 1);
  if (b >= 0 && b % 9 != 8) relevant_mask |= FillBits(b + 1);
  if (b >= 9) relevant_mask |= FillBits(b - 9);
  if (b <= 35) relevant_mask |= FillBits(b + 9);
  return relevant_mask;
}

constexpr uint64 GenerateColRelevantBits(int col) {
  return FillBits(col, col + 9, col + 18, col + 27, col + 36);
}

}  // namespace impl

inline uint64 HalfBitBoard::GetElephantOccupancy(Position pos) const {
  const int b = pos.value();
  static constexpr auto relevant_bits =
      GenerateArray<uint64, 45>(impl::GenerateElephantRelevantBits);
  return GatherBits(relevant_bits[b], FillBits(0, 17), b + 7,
                    static_cast<uint64>(15));
}

inline uint64 HalfBitBoard::GetHorseOccupancy(int pos) const {
  static constexpr auto relevant_bits =
      GenerateArray<uint64, 63>(impl::GenerateHorseRelevantBits);
  // Magic is shifted by 3 bits because pos could be -9 at minimum.
  return GatherBits(relevant_bits[pos + 9], FillBits(3, 10, 19), pos + 9,
                    static_cast<uint64>(15));
}

inline uint64 HalfBitBoard::GetRowOccupancy(int row) const {
  static constexpr uint64 row_mask = (1 << kNumColumns) - 1;
  return (value_ >> (row * kNumColumns)) & row_mask;
}

inline uint64 HalfBitBoard::GetColOccupancy(int col) const {
  static constexpr uint64 magic = FillBits(0, 8, 16, 24, 32);
  static constexpr auto col_relevant_bits =
      GenerateArray<uint64, kNumColumns>(impl::GenerateColRelevantBits);
  return GatherBits(col_relevant_bits[col], magic, col + 32,
                    static_cast<uint64>(31));
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
  return pos.InRedHalf()
             ? BitBoard(HalfBitBoard::Fill(pos), HalfBitBoard::EmptyBoard())
             : BitBoard(HalfBitBoard::EmptyBoard(),
                        HalfBitBoard::Fill(Position(pos.value() - 45)));
}

void BitBoard::Make(Move move) { *this ^= Fill(move.from()) | Fill(move.to()); }

void BitBoard::Unmake(Move move) { Make(move); }

uint64 BitBoard::GetElephantOccupancy(Position pos) const {
  return pos.InRedHalf()
             ? halves_[0].GetElephantOccupancy(pos)
             : halves_[1].GetElephantOccupancy(Position(pos.value() - 45));
}

uint64 BitBoard::GetHorseOccupancy(Position pos) const {
  uint64 ans = 0;
  if (pos.value() <= 53) ans |= halves_[0].GetHorseOccupancy(pos.value());
  if (pos.value() >= 36) ans |= halves_[1].GetHorseOccupancy(pos.value() - 45);
  return ans;
}

uint64 BitBoard::GetRowOccupancy(int row) const {
  return row < 5 ? halves_[0].GetRowOccupancy(row)
                 : halves_[1].GetRowOccupancy(row - 5);
}

uint64 BitBoard::GetColOccupancy(int col) const {
  return halves_[0].GetColOccupancy(col) |
         (halves_[1].GetColOccupancy(col) << 5);
}

bool BitBoard::operator[](Position pos) const {
  if (pos.value() < 45) {
    return halves_[0][pos.value()];
  } else {
    return halves_[1][pos.value() - 45];
  }
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

}  // namespace blur

#endif  // BLUR_BITBOARD_H
