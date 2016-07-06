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
  class Iterator {
   public:
    // Construct an uninitialized iterator.
    Iterator() = default;

    Iterator& operator++() {
      value_ &= value_ - 1;
      return *this;
    }
    Position operator*() const { return Position(lsb(value_)); }

    Iterator(const Iterator&) = default;
    Iterator& operator=(const Iterator&) = default;

    friend bool operator==(const Iterator i1, const Iterator i2) {
      return i1.value_ == i2.value_;
    }
    friend bool operator!=(const Iterator i1, const Iterator i2) {
      return !(i1 == i2);
    }

   private:
    Iterator(uint64 value) : value_(value) {}
    friend class HalfBitBoard;

    uint64 value_;
  };

  // An uninitialized HalfBitBoard.
  HalfBitBoard() = default;
  constexpr static HalfBitBoard EmptyBoard();
  // Returns a HalfBitBoard that has only the specified position set.
  // Requires: position.value() < 45.
  constexpr static HalfBitBoard Fill(Position position);

  inline void Move(Move move);

  // Returns lower 4 bits. From least significant: BL, TL, BR, TR.
  inline uint64 GetElephantOccupancy(Position pos) const;
  // Returns lower 4 bits. From least significant: L, B, R, T.
  // Requires pos range: [-9, 53].
  inline uint64 GetHorseOccupancy(int pos) const;
  inline uint64 GetRowOccupancy(int row) const;
  inline uint64 GetColOccupancy(int col) const;

  // Iterate over all set positions on this HalfBitBoard.
  Iterator begin() const { return Iterator(value_); }
  Iterator end() const { return Iterator(0); }

  inline HalfBitBoard(const HalfBitBoard&) = default;
  inline HalfBitBoard& operator=(const HalfBitBoard&) = default;

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
  class Iterator {
   public:
    // Construct an uninitialized iterator.
    Iterator() = default;

    Iterator& operator++() {
      ++iterators_[0];
      Canonicalize();
      return *this;
    }
    Position operator*() const {
      int original_value = (*(iterators_[0])).value();
      return current_ == 0 ? Position(original_value)
                           : Position(original_value + 45);
    }

    Iterator(const Iterator&) = default;
    Iterator& operator=(const Iterator&) = default;

    friend bool operator==(const Iterator i1, const Iterator i2) {
      return i1.current_ == i2.current_ && i1.iterators_[0] == i2.iterators_[0];
    }
    friend bool operator!=(const Iterator i1, const Iterator i2) {
      return !(i1 == i2);
    }

   private:
    using HalfIterator = HalfBitBoard::Iterator;

    Iterator(HalfIterator lower_iter_begin, HalfIterator upper_iter_begin,
             HalfIterator end)
        : current_(0), iterators_{lower_iter_begin, upper_iter_begin, end} {
      Canonicalize();
    }

    void Canonicalize() {
      while (current_ < 2 && iterators_[0] == iterators_[2]) {
        ++current_;
        iterators_[0] = iterators_[current_];
      }
    }

    friend class BitBoard;

    int current_;
    // 0 stores current iterator, 2 stores end.
    HalfIterator iterators_[3];
  };

  // An uninitialized BitBoard.
  BitBoard() = default;
  constexpr static BitBoard EmptyBoard();
  constexpr static BitBoard Fill(Position position);

  inline void Move(Move move);

  // Returns lower 4 bits. From least significant: BL, TL, BR, TR.
  inline uint64 GetElephantOccupancy(Position pos) const;
  // Returns lower 4 bits. From least significant: L, B, R, T.
  inline uint64 GetHorseOccupancy(Position pos) const;
  inline uint64 GetRowOccupancy(int row) const;
  inline uint64 GetColOccupancy(int col) const;

  // Iterate over all set positions on this BitBoard.
  Iterator begin() const {
    return Iterator(halves_[0].begin(), halves_[1].begin(), halves_[1].end());
  }
  Iterator end() const {
    return Iterator(halves_[0].end(), halves_[1].end(), halves_[1].end());
  }

  HalfBitBoard lower() const {
    return halves_[0];
  }
  HalfBitBoard upper() const {
    return halves_[1];
  }

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
