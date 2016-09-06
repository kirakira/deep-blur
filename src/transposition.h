#ifndef BLUR_TRANSPOSITION_H
#define BLUR_TRANSPOSITION_H

#include <array>
#include <atomic>
#include <utility>
#include <vector>

#include "common.h"
#include "board-base.h"

namespace blur {

enum class ScoreType : int { kExact = 0, kUpperBound, kLowerBound };

struct TTEntry {
  Score score;
  ScoreType type;
  Move best_move;
  int depth;

  static uint64 Serialize(TTEntry entry);
  static TTEntry Deserialize(uint64 value);
};

// Thread-safe.
class TranspositionTable {
 public:
  // Size of tt = 2^(5 + depth) bytes.
  explicit TranspositionTable(int depth = 22);

  // hash: sided hash of the position to look-up.
  // Returns true if the position is found.
  bool LookUp(uint64 hash, TTEntry* entry /* out */);
  void Store(uint64 hash, TTEntry entry);

  static bool IsLockFree();

 private:
  // Thread-safe.
  class KeyValuePair {
   public:
    std::pair<uint64, uint64> Load() const;
    void Store(uint64 key, uint64 value);

   private:
    std::atomic<uint64> key_;
    std::atomic<uint64> value_;
  };

  struct HashLine {
    std::array<KeyValuePair, 2> pairs;
  };

  const uint64 mask_;
  std::vector<HashLine> table_;
};

}  // namespace blur
#endif
