#include "transposition.h"

namespace blur {

// score: 16 bits signed   [48-63]
// type: 2 bits            [46-47]
// best_move: 14 bits      [32-45]
// depth: 10 bits unsigned [22-31]
uint64 TTEntry::Serialize(TTEntry entry) {
  return static_cast<uint64>(static_cast<std::int64_t>(entry.score) << 48) |
         (static_cast<uint64>(entry.type) << 46) |
         (static_cast<uint64>(entry.best_move.value()) << 32) |
         (static_cast<uint64>(entry.depth) << 22);
}

TTEntry TTEntry::Deserialize(uint64 value) {
  TTEntry entry;
  entry.score = static_cast<Score>(static_cast<std::int64_t>(value) >> 48);
  entry.type = static_cast<ScoreType>((value >> 46) & 3);
  entry.best_move = Move((value >> 32) & ((static_cast<uint64>(1) << 14) - 1));
  entry.depth =
      static_cast<int>((value >> 22) & ((static_cast<uint64>(1) << 10) - 1));
  return entry;
}

namespace {

// entry1 is considered more accurate than entry 2 if
// * entry1's depth is deeper than entry2, or
// * entry1's depth is equal to entry2's, but entry1 is exact while entry2 is
//   not.
bool MoreAccurateThan(TTEntry entry1, TTEntry entry2) {
  if (entry1.depth > entry2.depth) return true;
  if (entry1.depth < entry2.depth) return false;
  return (entry1.type == ScoreType::kExact && entry2.type != ScoreType::kExact);
}

}  // namespace

std::pair<uint64, uint64> TranspositionTable::KeyValuePair::Load() const {
  uint64 loaded_key = key_.load(std::memory_order_relaxed),
         loaded_value = value_.load(std::memory_order_relaxed);
  return std::make_pair(loaded_key ^ loaded_value, loaded_value);
}

void TranspositionTable::KeyValuePair::Store(uint64 key, uint64 value) {
  key_.store(key ^ value, std::memory_order_relaxed);
  value_.store(value, std::memory_order_relaxed);
}

TranspositionTable::TranspositionTable(int depth)
    : mask_((static_cast<uint64>(1) << depth) - 1), table_(1 << depth) {
  static_assert(sizeof(HashLine) == 32, "Size of HashLine does is not 32");
}

bool TranspositionTable::LookUp(uint64 key, TTEntry* entry) {
  for (const auto& key_val_pair : table_[key & mask_].pairs) {
    uint64 loaded_key, value;
    std::tie(loaded_key, value) = key_val_pair.Load();
    if (key == loaded_key) {
      *entry = TTEntry::Deserialize(value);
      return true;
    }
  }
  return false;
}

// Replacement policy:
// * If there already exists an entry for key, replace it if the new entry is
//   more accurate.
// * Otherwise, replace the entry with shallowest depth.
void TranspositionTable::Store(uint64 key, TTEntry entry) {
  const uint64 value = TTEntry::Serialize(entry);
  const size_t index = key & mask_;
  size_t index_to_replace = 0;
  int lowest_depth = 2048;
  for (size_t i = 0; i < table_[index].pairs.size(); ++i) {
    uint64 current_key, current_value;
    std::tie(current_key, current_value) = table_[index].pairs[i].Load();
    const auto current_entry = TTEntry::Deserialize(current_value);
    if (current_key == key) {
      // Don't overwrite if current value is more accurate than the value to
      // overwrite.
      if (MoreAccurateThan(current_entry, entry)) {
        return;
      } else {
        index_to_replace = i;
        break;
      }
    } else if (current_entry.depth <= lowest_depth) {
      lowest_depth = current_entry.depth;
      index_to_replace = i;
    }
  }
  table_[index].pairs[index_to_replace].Store(key, value);
}

/* static */
bool TranspositionTable::IsLockFree() {
  return std::atomic<uint64>().is_lock_free();
}

}  // namespace blur