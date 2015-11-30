#ifndef BLUR_BITBOARD_H
#define BLUR_BITBOARD_H

#include <iostream>

#include "board-common.h"
#include "common.h"

namespace blur {

// Value semantics.
class HalfBitBoard final {
 public:
  HalfBitBoard() = delete;
  constexpr static HalfBitBoard EmptyBoard() noexcept;
  // Returns a HalfBitBoard that has only the specified position set.
  // Requires: squares.value() < 45.
  constexpr static HalfBitBoard Fill(Position square) noexcept;

  void Move(Move move) noexcept;

  HalfBitBoard(const HalfBitBoard&) = default;
  HalfBitBoard& operator=(const HalfBitBoard&) = default;

  friend constexpr HalfBitBoard operator~(HalfBitBoard b) noexcept;
  friend constexpr HalfBitBoard operator&(HalfBitBoard b1,
                                          HalfBitBoard b2) noexcept;
  friend constexpr HalfBitBoard operator|(HalfBitBoard b1,
                                          HalfBitBoard b2) noexcept;
  friend constexpr HalfBitBoard operator^(HalfBitBoard b1,
                                          HalfBitBoard b2) noexcept;
  HalfBitBoard& operator&=(HalfBitBoard b) noexcept;
  HalfBitBoard& operator|=(HalfBitBoard b) noexcept;
  HalfBitBoard& operator^=(HalfBitBoard b) noexcept;
  friend constexpr bool operator==(HalfBitBoard b1, HalfBitBoard b2) noexcept;
  friend constexpr bool operator!=(HalfBitBoard b1, HalfBitBoard b2) noexcept;

  // For debug output only.
  friend std::ostream& operator<<(std::ostream& os, HalfBitBoard b);

 private:
  constexpr explicit HalfBitBoard(uint64 value) noexcept : value_(value) {}

  uint64 value_;
};

// Value semantics.
class BitBoard final {
 public:
  constexpr static BitBoard EmptyBoard() noexcept;
  constexpr static BitBoard Fill(Position pos) noexcept;

  void MakeMove(Move move) noexcept;

  BitBoard(const BitBoard&) = default;
  BitBoard& operator=(const BitBoard&) = default;

  friend constexpr BitBoard operator~(BitBoard b) noexcept;
  friend constexpr BitBoard operator&(BitBoard b1, BitBoard b2) noexcept;
  friend constexpr BitBoard operator|(BitBoard b1, BitBoard b2) noexcept;
  friend constexpr BitBoard operator^(BitBoard b1, BitBoard b2) noexcept;
  BitBoard& operator&=(BitBoard b) noexcept;
  BitBoard& operator|=(BitBoard b) noexcept;
  BitBoard& operator^=(BitBoard b) noexcept;
  friend constexpr bool operator==(BitBoard b1, BitBoard b2) noexcept;
  friend constexpr bool operator!=(BitBoard b1, BitBoard b2) noexcept;

  // For debug output only.
  friend std::ostream& operator<<(std::ostream& os, BitBoard b);

 private:
  constexpr BitBoard(HalfBitBoard lower, HalfBitBoard upper) noexcept
      : halves_{lower, upper} {}

  HalfBitBoard halves_[2];
};

// Lookup tables related to bitboard.
class BitTables final {
 public:
  constexpr BitTables() noexcept;

  BitBoard red_pawn_moves[90];
  BitBoard black_pawn_moves[90];

 private:
  void FillRedPawnMoves() noexcept;
  void FillBlackPawnMoves() noexcept;
};

constexpr BitTables kBitTables{};

}  // namespace blur

#endif // BLUR_BITBOARD_H
