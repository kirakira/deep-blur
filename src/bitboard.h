#ifndef BLUR_BITBOARD_H
#define BLUR_BITBOARD_H

#include <array>
#include <iostream>

#include "board-common.h"
#include "common.h"

namespace blur {

// Value semantics.
class HalfBitBoard {
 public:
  constexpr static HalfBitBoard EmptyBoard();
  // Returns a HalfBitBoard that has only the specified position set.
  // Requires: squares.value() < 45.
  constexpr static HalfBitBoard Fill(Position square);

  inline void Move(Move move);

  inline HalfBitBoard(const HalfBitBoard&) = default;
  inline HalfBitBoard& operator=(const HalfBitBoard&) = default;

  friend inline HalfBitBoard operator~(HalfBitBoard b);
  friend inline HalfBitBoard operator&(HalfBitBoard b1, HalfBitBoard b2);
  friend constexpr HalfBitBoard operator|(HalfBitBoard b1, HalfBitBoard b2);
  friend inline HalfBitBoard operator^(HalfBitBoard b1, HalfBitBoard b2);
  inline HalfBitBoard& operator&=(HalfBitBoard b);
  inline HalfBitBoard& operator|=(HalfBitBoard b);
  inline HalfBitBoard& operator^=(HalfBitBoard b);
  friend inline bool operator==(HalfBitBoard b1, HalfBitBoard b2);
  friend inline bool operator!=(HalfBitBoard b1, HalfBitBoard b2);

  // For debug output only.
  friend inline std::ostream& operator<<(std::ostream& os, HalfBitBoard b);

 private:
  constexpr explicit HalfBitBoard(uint64 value) : value_(value) {}

  uint64 value_;
};

// Value semantics.
class BitBoard {
 public:
  constexpr static BitBoard EmptyBoard();
  constexpr static BitBoard Fill(Position pos);

  inline void MakeMove(Move move);

  inline BitBoard(const BitBoard&) = default;
  inline BitBoard& operator=(const BitBoard&) = default;

  friend inline BitBoard operator~(BitBoard b);
  friend inline BitBoard operator&(BitBoard b1, BitBoard b2);
  friend constexpr BitBoard operator|(BitBoard b1, BitBoard b2);
  friend inline BitBoard operator^(BitBoard b1, BitBoard b2);
  inline BitBoard& operator&=(BitBoard b);
  inline BitBoard& operator|=(BitBoard b);
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

#include "bitboard-impl.h"

// Lookup tables related to bitboard.
class BitTables {
 public:
  static constexpr std::array<BitBoard, kNumPositions> red_pawn_moves =
      GenerateArray<BitBoard, kNumPositions>(RedPawnMovesAt);
  static constexpr std::array<BitBoard, kNumPositions> black_pawn_moves =
      GenerateArray<BitBoard, kNumPositions>(BlackPawnMovesAt);
  static constexpr std::array<BitBoard, kNumPositions> assistant_moves =
      GenerateArray<BitBoard, kNumPositions>(AssistantMovesAt);

 private:
  BitTables() = delete;
};

}  // namespace blur

#endif  // BLUR_BITBOARD_H
