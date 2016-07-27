#include "board-hash.h"

namespace blur {

constexpr uint64 BoardHash::piece_position_hash[16][kNumPositions];
constexpr uint64 BoardHash::side_hash;

}  // namespace blur
