#include <iostream>

#include "transposition.h"

using namespace std;
using namespace blur;

bool Equal(TTEntry e1, TTEntry e2) {
  if (e1.score != e2.score) return false;
  if (e1.type != e2.type) return false;
  if (e1.best_move.ToString() != e2.best_move.ToString()) return false;
  if (e1.depth != e2.depth) return false;
  return true;
}

bool SimpleTest() {
  if (!TranspositionTable::IsLockFree()) return false;
  TranspositionTable tt(1);
  TTEntry entry1;
  if (tt.LookUp(123, &entry1)) return false;
  entry1.score = static_cast<Score>(32767);
  entry1.type = ScoreType::kUpperBound;
  entry1.best_move = Move("c0c1");
  entry1.depth = 10;
  tt.Store(123, entry1);
  TTEntry stored_entry;
  if (!tt.LookUp(123, &stored_entry)) return false;
  if (!Equal(entry1, stored_entry)) return false;
  if (tt.LookUp(122, &stored_entry)) return false;
  if (tt.LookUp(125, &stored_entry)) return false;
  // Though hashed to the same bucket, entry2 should coexist with entry1.
  TTEntry entry2 = entry1;
  entry2.depth = 12;
  tt.Store(125, entry2);
  if (!tt.LookUp(125, &stored_entry)) return false;
  if (!Equal(entry2, stored_entry)) return false;
  if (!tt.LookUp(123, &stored_entry)) return false;
  if (!Equal(entry1, stored_entry)) return false;
  // Entry1 should be evicted.
  TTEntry entry3 = entry1;
  entry3.depth = 1023;
  tt.Store(127, entry3);
  if (tt.LookUp(123, &stored_entry)) return false;
  if (!tt.LookUp(127, &stored_entry)) return false;
  if (!Equal(stored_entry, entry3)) return false;
  return true;
}

bool NegativeScoreTest() {
  TranspositionTable tt;
  TTEntry entry;
  entry.score = static_cast<Score>(-32768);
  entry.type = ScoreType::kUpperBound;
  entry.best_move = Move("c0c1");
  entry.depth = 10;
  tt.Store(456, entry);
  TTEntry stored_entry;
  if (!tt.LookUp(456, &stored_entry)) return false;
  if (!Equal(entry, stored_entry)) return false;
  return true;
}

bool ReplacementTest() {
  TranspositionTable tt;
  TTEntry entry1;
  entry1.score = Score::kDrawScore;
  entry1.type = ScoreType::kUpperBound;
  entry1.best_move = Move("c0c1");
  entry1.depth = 10;
  tt.Store(456, entry1);
  TTEntry stored_entry;
  if (!tt.LookUp(456, &stored_entry)) return false;
  if (!Equal(entry1, stored_entry)) return false;
  // entry2 should replace entry1 because it has higher depth.
  TTEntry entry2 = entry1;
  entry2.depth = 12;
  tt.Store(456, entry2);
  if (!tt.LookUp(456, &stored_entry)) return false;
  if (!Equal(entry2, stored_entry)) return false;
  // entry3 should replace entry2 because it has an exact score.
  TTEntry entry3 = entry2;
  entry3.type = ScoreType::kExact;
  tt.Store(456, entry3);
  if (!tt.LookUp(456, &stored_entry)) return false;
  if (!Equal(entry3, stored_entry)) return false;
  // entry4 should be discarded because it has a lower depth.
  TTEntry entry4 = entry3;
  entry4.depth = 11;
  tt.Store(456, entry4);
  if (!tt.LookUp(456, &stored_entry)) return false;
  if (!Equal(entry3, stored_entry)) return false;
  // entry5 should be discarded because it has an inexact score.
  TTEntry entry5 = entry3;
  entry5.type = ScoreType::kLowerBound;
  if (!tt.LookUp(456, &stored_entry)) return false;
  if (!Equal(entry3, stored_entry)) return false;
  return true;
}

int main() {
  bool success = true;
  success = SimpleTest() && success;
  success = NegativeScoreTest() && success;
  success = ReplacementTest() && success;
  cout << (success ? "Success" : "Failed") << "." << endl;
  return success ? 0 : 1;
}
