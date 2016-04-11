#include "bitboard.h"

namespace blur {

constexpr std::array<BitBoard, kNumPositions> BitTables::red_pawn_moves;
constexpr std::array<BitBoard, kNumPositions> BitTables::black_pawn_moves;
constexpr std::array<BitBoard, kNumPositions> BitTables::king_moves;
constexpr std::array<BitBoard, kNumPositions> BitTables::assistant_moves;
constexpr std::array<std::array<BitBoard, 16>, kNumPositions>
    BitTables::elephant_moves;
constexpr std::array<std::array<BitBoard, 16>, kNumPositions>
    BitTables::horse_moves;
constexpr std::array<std::array<BitBoard, 512>, kNumPositions>
    BitTables::cannon_row_moves;
constexpr std::array<std::array<BitBoard, 1024>, kNumPositions>
    BitTables::cannon_col_moves;

}  // namespace blur
