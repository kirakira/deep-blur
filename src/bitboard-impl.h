#ifdef BLUR_BITBOARD_H  // Requires including bitboard.h.

#include <utility>

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

inline uint64 HalfBitBoard::GetElephantOccupancy(Position pos) const {
  const int b = pos.value();
  uint64 relevant_mask;
  if (b >= 10) {
    relevant_mask = FillBits(b - 10, b - 8, b + 8, b + 10);
  } else if (b >= 8) {
    relevant_mask = FillBits(b - 8, b + 8, b + 10);
  } else {
    relevant_mask = FillBits(b + 8, b + 10);
  }
  return GatherBits(relevant_mask, FillBits(0, 17), b + 7,
                    static_cast<uint64>(15));
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

inline uint64 HalfBitBoard::GetHorseOccupancy(int pos) const {
  static constexpr auto relevant_bits =
      GenerateArray<uint64, 63>(GenerateHorseRelevantBits);
  // Magic is shifted by 3 bits because pos could be -9 at minimum.
  return GatherBits(relevant_bits[pos + 9], FillBits(3, 10, 19), pos + 9,
                    static_cast<uint64>(15));
}

inline uint64 HalfBitBoard::GetRowOccupancy(int row) const {
  static constexpr uint64 row_mask = (1 << kNumColumns) - 1;
  return (value_ >> (row * kNumColumns)) & row_mask;
}

constexpr uint64 GenerateColRelevantBits(int col) {
  return FillBits(col, col + 9, col + 18, col + 27, col + 36);
}

inline uint64 HalfBitBoard::GetColOccupancy(int col) const {
  static constexpr uint64 magic = FillBits(0, 8, 16, 24, 32);
  static constexpr auto col_relevant_bits =
      GenerateArray<uint64, kNumColumns>(GenerateColRelevantBits);
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

namespace impl {

//   ---
//   |1|
// -------
// |2|*|0|
// -------
//   |3|
//   ---
constexpr int kAdjacentOffsets[][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

// --- ---
// |1| |0|
// -------
//   |*|
// -------
// |2| |3|
// --- ---
constexpr int kDiagonalOffsets[][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};

constexpr std::array<std::array<int, 3>, 4> kElephantMovePattern = {{
    {{2, 2, 3}}, {{2, -2, 1}}, {{-2, -2, 0}}, {{-2, 2, 2}},
}};  // I agree this is stupid.

// -----------
// | |2| |1| |
// -----------
// |3| | | |0|
// -----------
// | | |*| | |
// -----------
// |4| | | |7|
// -----------
// | |5| |6| |
// -----------
constexpr std::array<std::array<int, 3>, 8> kHorseMovePattern = {
    {{{1, 2, 2}},
     {{2, 1, 3}},
     {{2, -1, 3}},
     {{1, -2, 0}},
     {{-1, -2, 0}},
     {{-2, -1, 1}},
     {{-2, 1, 1}},
     {{-1, 2, 2}}}};

constexpr bool InBoard(int i, int j) { return Position::IsValidPosition(i, j); }

constexpr bool InRedHalf(int i, int j) {
  return InBoard(i, j) && Position(i, j).InRedHalf();
}

constexpr bool InBlackHalf(int i, int j) {
  return InBoard(i, j) && !InRedHalf(i, j);
}

constexpr bool InRedPalace(int i, int j) {
  return InBoard(i, j) && i <= 2 && j >= 3 && j <= 5;
}

constexpr bool InBlackPalace(int i, int j) {
  return InBoard(i, j) && i >= 7 && j >= 3 && j <= 5;
}

// Sometimes we don't care red or black...
constexpr bool InPalace(int i, int j) {
  return InRedPalace(i, j) || InBlackPalace(i, j);
}

template <typename... Index>
constexpr std::array<std::array<int, 2>, sizeof...(Index)> MakeOffsets(
    const int offset_array[][2], Index... indices) {
  return {{{{offset_array[indices][0], offset_array[indices][1]}}...}};
}

template <typename Predicate, size_t offset_length>
constexpr BitBoard RelativePositions(
    int current_i, int current_j, Predicate predicate,
    const std::array<std::array<int, 2>, offset_length>& offsets) {
  auto result = BitBoard::EmptyBoard();
  // Cannot use range-based for-loop because stupid C++14 does not mark
  // std::array::begin() as constexpr.
  for (size_t i = 0; i < offsets.size(); ++i) {
    const int ni = current_i + offsets[i][0], nj = current_j + offsets[i][1];
    if (predicate(ni, nj)) {
      result |= BitBoard::Fill(Position(ni, nj));
    }
  }
  return result;
}

template <typename Predicate, size_t offsets_length>
constexpr BitBoard RelativePositionsWithOccupancy(
    int current_i, int current_j, uint64 occupancy, Predicate predicate,
    const std::array<std::array<int, 3>, offsets_length>& offsets) {
  auto result = BitBoard::EmptyBoard();
  for (size_t i = 0; i < offsets.size(); ++i) {
    const int ni = current_i + offsets[i][0], nj = current_j + offsets[i][1];
    if (predicate(ni, nj, GetBit(occupancy, offsets[i][2]))) {
      result |= BitBoard::Fill(Position(ni, nj));
    }
  }
  return result;
}

constexpr BitBoard RedPawnMovesAt(size_t index) {
  Position pos(index);
  int i = pos.Row(), j = pos.Column();
  if (InRedHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 1));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 1, 2));
  }
}

constexpr BitBoard BlackPawnMovesAt(size_t index) {
  Position pos(index);
  int i = pos.Row(), j = pos.Column();
  if (InBlackHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 3));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 2, 3));
  }
}

constexpr BitBoard KingMovesAt(size_t index) {
  Position pos(index);
  int i = pos.Row(), j = pos.Column();
  if (InPalace(i, j)) {
    return RelativePositions(i, j, InPalace,
                             MakeOffsets(kAdjacentOffsets, 0, 1, 2, 3));
  } else {
    return BitBoard::EmptyBoard();
  }
}

constexpr BitBoard AssistantMovesAt(size_t index) {
  Position pos(index);
  int i = pos.Row(), j = pos.Column();
  if (InPalace(i, j)) {
    return RelativePositions(i, j, InPalace,
                             MakeOffsets(kDiagonalOffsets, 0, 1, 2, 3));
  } else {
    return BitBoard::EmptyBoard();
  }
}

constexpr bool CheckRedElephantPosition(int i, int j, int occupancy_bit) {
  return !occupancy_bit && InRedHalf(i, j);
}

constexpr bool CheckBlackElephantPosition(int i, int j, int occupancy_bit) {
  return !occupancy_bit && InBlackHalf(i, j);
}

constexpr BitBoard ElephantMovesWithOccupancy(size_t index, uint64 occupancy) {
  Position pos(index);
  int i = pos.Row(), j = pos.Column();
  if (InRedHalf(i, j)) {
    return RelativePositionsWithOccupancy(
        i, j, occupancy, CheckRedElephantPosition, kElephantMovePattern);
  } else {
    return RelativePositionsWithOccupancy(
        i, j, occupancy, CheckBlackElephantPosition, kElephantMovePattern);
  }
}

constexpr auto ElephantMovesAt(size_t index) {
  return GenerateArray<BitBoard, 16>(
      CurryFront(ElephantMovesWithOccupancy, index));
}

constexpr bool CheckHorsePosition(int i, int j, int occupancy_bit) {
  return !occupancy_bit && InBoard(i, j);
}

constexpr BitBoard HorseMovesWithOccupancy(size_t index, uint64 occupancy) {
  Position pos(index);
  int i = pos.Row(), j = pos.Column();
  return RelativePositionsWithOccupancy(i, j, occupancy, CheckHorsePosition,
                                        kHorseMovePattern);
}

constexpr auto HorseMovesAt(size_t index) {
  return GenerateArray<BitBoard, 16>(
      CurryFront(HorseMovesWithOccupancy, index));
}

constexpr BitBoard CannonRowMovesWithOccupancy(size_t index, uint64 occupancy) {
  BitBoard moves = BitBoard::EmptyBoard();
  Position pos(index);
  const int i = pos.Row(), j = pos.Column();
  for (int dir = -1; dir <= 1; dir += 2) {
    int pieces_met = 0;
    for (int ni = i, nj = j + dir; InBoard(ni, nj) && pieces_met < 2;
         nj += dir) {
      const bool occupied = GetBit(occupancy, nj);
      if (occupied) {
        if (pieces_met == 1) {
          moves |= BitBoard::Fill(Position(ni, nj));
        }
        ++pieces_met;
      } else {
        if (pieces_met == 0) {
          moves |= BitBoard::Fill(Position(ni, nj));
        }
      }
    }
  }
  return moves;
}

constexpr auto CannonRowMovesAt(size_t index) {
  return GenerateArray<BitBoard, 512>(
      CurryFront(CannonRowMovesWithOccupancy, index));
}

constexpr BitBoard CannonColMovesWithOccupancy(size_t index, uint64 occupancy) {
  BitBoard moves = BitBoard::EmptyBoard();
  Position pos(index);
  const int i = pos.Row(), j = pos.Column();
  for (int dir = -1; dir <= 1; dir += 2) {
    int pieces_met = 0;
    for (int ni = i + dir, nj = j; InBoard(ni, nj) && pieces_met < 2;
         ni += dir) {
      const bool occupied = GetBit(occupancy, ni);
      if (occupied) {
        if (pieces_met == 1) {
          moves |= BitBoard::Fill(Position(ni, nj));
        }
        ++pieces_met;
      } else {
        if (pieces_met == 0) {
          moves |= BitBoard::Fill(Position(ni, nj));
        }
      }
    }
  }
  return moves;
}

constexpr auto CannonColMovesAt(size_t index) {
  return GenerateArray<BitBoard, 1024>(
      CurryFront(CannonColMovesWithOccupancy, index));
}

}  // namespace impl

#endif  // BLUR_BITBOARD_H
