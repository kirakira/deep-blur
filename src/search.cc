#include "search.h"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "logger.h"
#include "move-picker.h"

using std::string;
using std::vector;

namespace blur {

namespace {

// A scoped move maker.
class CheckedMoveMaker {
 public:
  CheckedMoveMaker(Board* board, Side side, Move move) : board_(board) {
    std::tie(move_made_, move_type_) = board->CheckedMake(side, move);
  }

  ~CheckedMoveMaker() {
    if (move_made_) board_->Unmake();
  }

  bool move_made() const { return move_made_; }
  MoveType move_type() const { return move_type_; }

 private:
  Board* const board_;
  bool move_made_;
  MoveType move_type_;
};

struct Stats {
  int64 nodes_visited = 0;
  int64 tt_hit = 0;
  int64 affected_by_history = 0;
  static constexpr int kBestMoveRankSize = 10;
  int64 best_move_index[kBestMoveRankSize] = {0};
  int64 best_move_total = 0;

  void IncrementBestMoveRank(int rank) {
    ++best_move_total;
    if (rank < kBestMoveRankSize) ++best_move_index[rank];
  }

  void Print() {
    std::cout << "# nodes: " << nodes_visited << ", tt hit: "
              << static_cast<double>(tt_hit) * 100 /
                     static_cast<double>(nodes_visited)
              << "%, affected by history: "
              << static_cast<double>(affected_by_history) * 100 /
                     static_cast<double>(nodes_visited)
              << "%" << std::endl;
    std::cout << "best move ranks: ";
    for (int i = 0; i < kBestMoveRankSize; ++i) {
      std::cout << static_cast<double>(best_move_index[i]) * 100 /
                       static_cast<double>(best_move_total)
                << "% ";
    }
    std::cout << std::endl;
  }
};

struct DebugInfo {
  int64 parent_id = -1;
  Move from_move;
};

enum DebugOptions { kDebugOn, kDebugOff };

template <DebugOptions debug_options>
struct SearchParams;

template <>
struct SearchParams<kDebugOff> {
  TranspositionTable* tt;
  KillerStats* killer_stats;
  Stats* stats;
};

#ifndef NDEBUG
template <>
struct SearchParams<kDebugOn> : public SearchParams<kDebugOff> {
  DebugInfo debug_info;
};
#endif

template <DebugOptions debug_options>
void DebugModifyChildParams(int64, Move, SearchParams<debug_options>*) {}

#ifndef NDEBUG
template <>
void DebugModifyChildParams<kDebugOn>(int64 node_id, Move move,
                                      SearchParams<kDebugOn>* params) {
  params->debug_info.parent_id = node_id;
  params->debug_info.from_move = move;
}
#endif

template <DebugOptions debug_options>
void DebugLogCurrentNode(int64, const SearchParams<debug_options>, Side,
                         const int, const Score, const Score, bool,
                         SearchResult) {}

#ifndef NDEBUG
template <>
void DebugLogCurrentNode<kDebugOn>(int64 node_id,
                                   const SearchParams<kDebugOn> params,
                                   Side side, const int depth,
                                   const Score alpha, const Score beta,
                                   bool tt_hit, SearchResult result) {
  Logger::SearchNodeInfo log_node_info(node_id, params.debug_info.parent_id,
                                       params.debug_info.from_move, side, depth,
                                       alpha, beta);
  log_node_info.tt_hit = tt_hit;
  log_node_info.score = result.score;
  log_node_info.best_move = result.best_move;
  Logger::GetInstance()->SaveNode(log_node_info);
}
#endif

struct InternalSearchResult {
  SearchResult external_result;
  bool affected_by_history = false;
};

// If it returns true, the move in result must be a valid move; if it returns
// false, tt_move could be corropted or invalid.
bool ProbeTT(Board* board, TranspositionTable* tt, Side side, int depth,
             Score alpha, Score beta, InternalSearchResult* result,
             Move* tt_move) {
  const uint64 hash = board->HashCode(side);
  TTEntry entry;
  if (!tt->LookUp(hash, &entry)) return false;
  if (entry.depth >= depth &&
      (entry.type == ScoreType::kExact ||
       (entry.type == ScoreType::kUpperBound && entry.score <= alpha) ||
       (entry.type == ScoreType::kLowerBound && entry.score >= beta))) {
    // Check if the move is valid.
    if (depth >= 1 && entry.score != -Score::kMateScore) {
      if (!entry.best_move.IsValid()) return false;
      CheckedMoveMaker move_maker(board, side, entry.best_move);
      if (!move_maker.move_made() ||
          move_maker.move_type() == MoveType::kPerpetualAttacker) {
        return false;
      }
    }
    result->external_result.score = entry.score;
    result->external_result.best_move = entry.best_move;
    result->affected_by_history = false;
    return true;
  }
  // If the tt result is unusable, we populate tt_move to inspire move ordering
  // in search.
  *tt_move = entry.best_move;
  return false;
}

void StoreTT(Board* board, TranspositionTable* tt, Side side, int depth,
             Score alpha, Score beta, SearchResult result) {
  const uint64 hash = board->HashCode(side);
  TTEntry entry;
  entry.score = result.score;
  if (result.score <= alpha) {
    entry.type = ScoreType::kUpperBound;
  } else if (result.score < beta) {
    entry.type = ScoreType::kExact;
  } else {
    entry.type = ScoreType::kLowerBound;
  }
  entry.best_move = result.best_move;
  entry.depth = depth;
  tt->Store(hash, entry);
}

Score ScoreFromRepetitionRule(MoveType type) {
  switch (type) {
    case MoveType::kRepetition:
      return Score::kDrawScore;
    case MoveType::kPerpetualAttacker:
      return -Score::kMateScore;
    case MoveType::kPerpetualAttackee:
      return Score::kMateScore;
    default:
      CHECK(false);
      return Score::kDrawScore;
  }
}

// Only scores within (alpha, beta) are exact. Scores <= alpha are upper bounds;
// scores >= beta are lower bounds.
template <DebugOptions debug_options>
InternalSearchResult Search(Board* const board, const Side side,
                            const int depth, const Score alpha,
                            const Score beta,
                            const SearchParams<debug_options> params) {
  const int64 node_id = params.stats->nodes_visited++;
  InternalSearchResult result;

  // Probe tt.
  bool tt_hit = false;
  Move tt_move;
  int best_move_index = -1;
  if (ProbeTT(board, params.tt, side, depth, alpha, beta, &result, &tt_move)) {
    ++params.stats->tt_hit;
    tt_hit = true;
  } else if (depth == 0) {
    Score score = board->Evaluation();
    result.external_result.score = (side == Side::kRed ? score : -score);
  } else {
    result.external_result.score = -kMateScore;
    int num_moves_tried = 0;
    for (const auto move :
         MovePicker(*board, side, tt_move, depth, params.killer_stats)) {
      CheckedMoveMaker move_maker(board, side, move);
      if (!move_maker.move_made()) continue;

      ++num_moves_tried;

      const Score current_alpha = std::max(result.external_result.score, alpha);

      InternalSearchResult child_result;
      if (move_maker.move_type() != MoveType::kRegular) {
        child_result.external_result.score =
            ScoreFromRepetitionRule(move_maker.move_type());
        // We won't have a best_move in this case.
        child_result.affected_by_history = true;
      } else {
        SearchParams<debug_options> child_params = params;
        DebugModifyChildParams(node_id, move, &child_params);
        child_result = Search(board, OtherSide(side), depth - 1, -beta,
                              -current_alpha, child_params);
      }
      const Score current_score = -child_result.external_result.score;

      if (current_score > result.external_result.score) {
        result.external_result.score = current_score;
        result.external_result.best_move = move;
        best_move_index = num_moves_tried - 1;
      }
      // Set affected_by_history as long as there's one child affected by
      // history.
      if (child_result.affected_by_history) {
        result.affected_by_history = true;
      }

      if (result.external_result.score >= beta) {
        params.killer_stats->RecordBetaCut(depth, move);
        // Reset affected_by_history if a beta-cutoff happens.
        result.affected_by_history = child_result.affected_by_history;
        break;
      }
    }
  }

  if (best_move_index != -1)
    params.stats->IncrementBestMoveRank(best_move_index);
  if (result.affected_by_history) ++params.stats->affected_by_history;
  // Store TT.
  if (!tt_hit && !result.affected_by_history) {
    StoreTT(board, params.tt, side, depth, alpha, beta, result.external_result);
  }

  DebugLogCurrentNode(node_id, params, side, depth, alpha, beta, tt_hit,
                      result.external_result);
  return result;
}

}  // namespace

SearchResult Search(Board* board, TranspositionTable* tt, Side side,
                    int depth) {
  board->ResetRepetitionHistory();
#ifdef NDEBUG
  constexpr DebugOptions debug = kDebugOff;
#else
  constexpr DebugOptions debug = kDebugOn;
#endif
  SearchParams<debug> params;
  Stats stats;
  params.stats = &stats;
  KillerStats killer_stats;
  params.killer_stats = &killer_stats;
  params.tt = tt;
  const auto result =
      Search(board, side, depth, -kMateScore, kMateScore, params);
  params.stats->Print();
  return result.external_result;
}

void DebugPrintLogs() { Logger::GetInstance()->Print(); }

}  // namespace blur
