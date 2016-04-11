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
  // Requires: position.value() < 45.
  constexpr static HalfBitBoard Fill(Position position);

  inline void Move(Move move);

  // Returns lower 4 bits. From least significant: BR, TL, BL, TR.
  inline uint64 GetElephantOccupancy(Position pos) const;
  // Returns lower 4 bits. From least significant: L, B, R, T.
  // Requires pos range: [-9, 53].
  inline uint64 GetHorseOccupancy(int pos) const;
  inline uint64 GetRowOccupancy(int row) const;
  inline uint64 GetColOccupancy(int col) const;

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
  constexpr static BitBoard EmptyBoard();
  constexpr static BitBoard Fill(Position position);

  inline void Move(Move move);

  // Returns lower 4 bits. From least significant: BR, TL, BL, TR.
  inline uint64 GetElephantOccupancy(Position pos) const;
  // Returns lower 4 bits. From least significant: L, B, R, T.
  inline uint64 GetHorseOccupancy(Position pos) const;
  inline uint64 GetRowOccupancy(int row) const;
  inline uint64 GetColOccupancy(int col) const;

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

#include "bitboard-impl.h"

// Lookup tables related to bitboard.
class BitTables {
 public:
  static constexpr std::array<BitBoard, kNumPositions> red_pawn_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::RedPawnMovesAt);
  static constexpr std::array<BitBoard, kNumPositions> black_pawn_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::BlackPawnMovesAt);
  static constexpr std::array<BitBoard, kNumPositions> king_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::KingMovesAt);
  static constexpr std::array<BitBoard, kNumPositions> assistant_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::AssistantMovesAt);
  // elephant_moves[pos][occupancy]. Occupancy ranges from [0, 2^4).
  // Bits order of occupancy from its least significant bit: BR, TL, BL, TR.
  static constexpr std::array<std::array<BitBoard, 16>, kNumPositions>
      elephant_moves = GenerateArray<std::array<BitBoard, 16>, kNumPositions>(
          impl::ElephantMovesAt);
  // horse_moves[pos][occupancy]. Occupancy ranges from [0, 2^4).
  // Bits order of occupancy from its least significant bit: L, B, R, T.
  static constexpr std::array<std::array<BitBoard, 16>, kNumPositions>
      horse_moves = GenerateArray<std::array<BitBoard, 16>, kNumPositions>(
          impl::HorseMovesAt);
  // cannon_row_moves[pos][row]. Row ranges from [0, 2^9) encoding 1 bit for
  // each position in the row (1 for occupied, 0 for unoccupied) from left to
  // right.
  static constexpr std::array<std::array<BitBoard, 512>, kNumPositions>
      cannon_row_moves =
          GenerateArray<std::array<BitBoard, 512>, kNumPositions>(
              impl::CannonRowMovesAt);
  // cannon_col_moves[pos][col]. Col ranges from [0, 2^10) encoding 1 bit for
  // each position in the column (1 for occupied, 0 for unoccupied) from down to
  // top.
  static constexpr std::array<std::array<BitBoard, 1024>, kNumPositions>
      cannon_col_moves =
          GenerateArray<std::array<BitBoard, 1024>, kNumPositions>(
              impl::CannonColMovesAt);

 private:
  BitTables() = delete;
};

}  // namespace blur

#endif  // BLUR_BITBOARD_H
