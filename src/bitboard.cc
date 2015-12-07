#include "bitboard.h"

namespace blur {

constexpr std::array<BitBoard, kNumPositions> BitTables::red_pawn_moves;
constexpr std::array<BitBoard, kNumPositions> BitTables::black_pawn_moves;
constexpr std::array<BitBoard, kNumPositions> BitTables::assistant_moves;

}  // namespace blur
