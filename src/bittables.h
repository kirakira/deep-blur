#ifndef BLUR_BITTABLES_H
#define BLUR_BITTABLES_H

#include "bitboard.h"

namespace blur {
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
constexpr std::array<std::array<int, 3>, 8> kHorseReverseMovePattern = {
    {{{1, 2, 3}},
     {{2, 1, 3}},
     {{2, -1, 1}},
     {{1, -2, 1}},
     {{-1, -2, 0}},
     {{-2, -1, 0}},
     {{-2, 1, 2}},
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
  Position pos(static_cast<int>(index));
  int i = pos.Row(), j = pos.Column();
  if (InRedHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 1));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 1, 2));
  }
}

constexpr BitBoard RedPawnReverseMovesAt(size_t index) {
  Position pos(static_cast<int>(index));
  int i = pos.Row(), j = pos.Column();
  if (InRedHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 3));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 2, 3));
  }
}

constexpr BitBoard BlackPawnMovesAt(size_t index) {
  Position pos(static_cast<int>(index));
  int i = pos.Row(), j = pos.Column();
  if (InBlackHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 3));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 2, 3));
  }
}

constexpr BitBoard BlackPawnReverseMovesAt(size_t index) {
  Position pos(static_cast<int>(index));
  int i = pos.Row(), j = pos.Column();
  if (InBlackHalf(i, j)) {
    return RelativePositions(i, j, InBoard, MakeOffsets(kAdjacentOffsets, 1));
  } else {
    return RelativePositions(i, j, InBoard,
                             MakeOffsets(kAdjacentOffsets, 0, 1, 2));
  }
}

constexpr BitBoard KingMovesAt(size_t index) {
  Position pos(static_cast<int>(index));
  int i = pos.Row(), j = pos.Column();
  if (InPalace(i, j)) {
    return RelativePositions(i, j, InPalace,
                             MakeOffsets(kAdjacentOffsets, 0, 1, 2, 3));
  } else {
    return BitBoard::EmptyBoard();
  }
}

constexpr BitBoard AssistantMovesAt(size_t index) {
  Position pos(static_cast<int>(index));
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
  Position pos(static_cast<int>(index));
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
  Position pos(static_cast<int>(index));
  int i = pos.Row(), j = pos.Column();
  return RelativePositionsWithOccupancy(i, j, occupancy, CheckHorsePosition,
                                        kHorseMovePattern);
}

constexpr auto HorseMovesAt(size_t index) {
  return GenerateArray<BitBoard, 16>(
      CurryFront(HorseMovesWithOccupancy, index));
}

constexpr BitBoard HorseReverseMovesWithOccupancy(size_t index,
                                                  uint64 occupancy) {
  Position pos(static_cast<int>(index));
  int i = pos.Row(), j = pos.Column();
  return RelativePositionsWithOccupancy(i, j, occupancy, CheckHorsePosition,
                                        kHorseReverseMovePattern);
}

constexpr auto HorseReverseMovesAt(size_t index) {
  return GenerateArray<BitBoard, 16>(
      CurryFront(HorseReverseMovesWithOccupancy, index));
}

constexpr BitBoard CannonRowMovesWithOccupancy(size_t column,
                                               uint64 occupancy) {
  BitBoard moves = BitBoard::EmptyBoard();
  const int i = 0, j = static_cast<int>(column);
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

constexpr auto CannonRowMovesAt(size_t column) {
  return GenerateArray<BitBoard, 512>(
      CurryFront(CannonRowMovesWithOccupancy, column));
}

constexpr BitBoard CannonColMovesWithOccupancy(size_t row, uint64 occupancy) {
  BitBoard moves = BitBoard::EmptyBoard();
  const int i = static_cast<int>(row), j = 0;
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

constexpr auto CannonColMovesAt(size_t row) {
  return GenerateArray<BitBoard, 1024>(
      CurryFront(CannonColMovesWithOccupancy, row));
}

constexpr auto RookRowMovesWithOccupancy(size_t column, uint64 occupancy) {
  BitBoard moves = BitBoard::EmptyBoard();
  const int i = 0, j = static_cast<int>(column);
  for (int dir = -1; dir <= 1; dir += 2) {
    for (int ni = i, nj = j + dir; InBoard(ni, nj); nj += dir) {
      moves |= BitBoard::Fill(Position(ni, nj));
      if (GetBit(occupancy, nj)) break;
    }
  }
  return moves;
}

constexpr auto RookRowMovesAt(size_t column) {
  return GenerateArray<BitBoard, 512>(
      CurryFront(RookRowMovesWithOccupancy, column));
}

constexpr auto RookColMovesWithOccupancy(size_t row, uint64 occupancy) {
  BitBoard moves = BitBoard::EmptyBoard();
  const int i = static_cast<int>(row), j = 0;
  for (int dir = -1; dir <= 1; dir += 2) {
    for (int ni = i + dir, nj = j; InBoard(ni, nj); ni += dir) {
      moves |= BitBoard::Fill(Position(ni, nj));
      if (GetBit(occupancy, ni)) break;
    }
  }
  return moves;
}

constexpr auto RookColMovesAt(size_t row) {
  return GenerateArray<BitBoard, 1024>(
      CurryFront(RookColMovesWithOccupancy, row));
}

}  // namespace impl

// Lookup tables related to bitboard.
class BitTables {
 public:
  constexpr BitTables() = default;

  const std::array<BitBoard, kNumPositions> red_pawn_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::RedPawnMovesAt);
  const std::array<BitBoard, kNumPositions> red_pawn_reverse_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::RedPawnReverseMovesAt);
  const std::array<BitBoard, kNumPositions> black_pawn_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::BlackPawnMovesAt);
  const std::array<BitBoard, kNumPositions> black_pawn_reverse_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::BlackPawnReverseMovesAt);
  const std::array<BitBoard, kNumPositions> king_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::KingMovesAt);
  const std::array<BitBoard, kNumPositions> assistant_moves =
      GenerateArray<BitBoard, kNumPositions>(impl::AssistantMovesAt);
  // elephant_moves[pos][occupancy]. Occupancy ranges from [0, 2^4).
  // Bits order of occupancy from its least significant bit: BL, TL, BR, TR.
  const std::array<std::array<BitBoard, 16>, kNumPositions> elephant_moves =
      GenerateArray<std::array<BitBoard, 16>, kNumPositions>(
          impl::ElephantMovesAt);
  // horse_moves[pos][occupancy]. Occupancy ranges from [0, 2^4).
  // Bits order of occupancy from its least significant bit: L, B, R, T.
  const std::array<std::array<BitBoard, 16>, kNumPositions> horse_moves =
      GenerateArray<std::array<BitBoard, 16>, kNumPositions>(
          impl::HorseMovesAt);
  // horse_reverse_moves[pos][occupancy]. Occupancy ranges from [0, 2^4).
  // Occupancy same as elephant occupancy: BL, TL, BR, TR.
  const std::array<std::array<BitBoard, 16>, kNumPositions>
      horse_reverse_moves =
          GenerateArray<std::array<BitBoard, 16>, kNumPositions>(
              impl::HorseReverseMovesAt);
  // cannon_row_moves[col][row_occ]. Row occupancy ranges from [0, 2^9) encoding
  // 1 bit for each position in the row (1 for occupied, 0 for unoccupied) from
  // left to right.
  const std::array<std::array<BitBoard, 512>, kNumColumns> cannon_row_moves =
      GenerateArray<std::array<BitBoard, 512>, kNumColumns>(
          impl::CannonRowMovesAt);
  // cannon_col_moves[row][col_occ]. Col occupancy ranges from [0, 2^10)
  // encoding 1 bit for each position in the column (1 for occupied, 0 for
  // unoccupied) from down to top.
  const std::array<std::array<BitBoard, 1024>, kNumRows> cannon_col_moves =
      GenerateArray<std::array<BitBoard, 1024>, kNumRows>(
          impl::CannonColMovesAt);
  // rook_row_moves[col][row_occ]. Row occupancy ranges from [0, 2^9) encoding 1
  // bit for each position in the row (1 for occupied, 0 for unoccupied) from
  // left to right.
  const std::array<std::array<BitBoard, 512>, kNumColumns> rook_row_moves =
      GenerateArray<std::array<BitBoard, 512>, kNumColumns>(
          impl::RookRowMovesAt);
  // rook_col_moves[row][col_occ]. Col occupancy ranges from [0, 2^10) encoding
  // 1 bit for each position in the column (1 for occupied, 0 for unoccupied)
  // from down to top.
  const std::array<std::array<BitBoard, 1024>, kNumRows> rook_col_moves =
      GenerateArray<std::array<BitBoard, 1024>, kNumRows>(impl::RookColMovesAt);
};

}  // namespace blur

#endif  // BLUR_BITTABLES_H
