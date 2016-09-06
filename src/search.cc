#include "search.h"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "logger.h"

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
  int64 repetition_detected = 0;
  int64 tt_hit = 0;

  void Print() {
    std::cout << "# nodes: " << nodes_visited << " tt hit: "
              << static_cast<double>(tt_hit) * 100 / nodes_visited
              << "%, # repetitions: " << repetition_detected << std::endl;
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
struct SearchParams<kDebugOff> {};

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

bool ProbeTT(Board* board, TranspositionTable* tt, Side side, int depth,
             Score alpha, Score beta, SearchResult* result) {
  const uint64 hash = board->HashCode(side);
  TTEntry entry;
  if (!tt->LookUp(hash, &entry)) return false;
  if (entry.depth >= depth &&
      (entry.type == ScoreType::kExact ||
       (entry.type == ScoreType::kUpperBound && entry.score <= alpha) ||
       (entry.type == ScoreType::kLowerBound && entry.score >= beta))) {
    // Check if the move is valid.
    if (depth >= 1 && entry.score != -Score::kMateScore) {
      CheckedMoveMaker move_maker(board, side, entry.best_move);
      if (!move_maker.move_made() ||
          move_maker.move_type() == MoveType::kPerpetualAttacker) {
        return false;
      }
    }
    result->score = entry.score;
    result->best_move = entry.best_move;
    return true;
  }
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

// Only scores within (alpha, beta) are exact. Scores <= alpha are upper bounds;
// scores >= beta are lower bounds.
template <DebugOptions debug_options>
SearchResult Search(Board* const board, TranspositionTable* tt, const Side side,
                    const int depth, const Score alpha, const Score beta,
                    Stats* const stats,
                    const SearchParams<debug_options> params) {
  const int64 node_id = (stats->nodes_visited)++;
  SearchResult result;

  // Probe tt.
  bool tt_hit = false;
  if (ProbeTT(board, tt, side, depth, alpha, beta, &result)) {
    ++(stats->tt_hit);
    tt_hit = true;
  } else if (depth == 0) {
    Score score = board->Evaluation();
    result.score = (side == Side::kRed ? score : -score);
  } else {
    result.score = -kMateScore;
    for (const auto move : board->GenerateMoves(side)) {
      CheckedMoveMaker move_maker(board, side, move);
      if (!move_maker.move_made()) continue;
      if (move_maker.move_type() == MoveType::kRepetition ||
          move_maker.move_type() == MoveType::kPerpetualAttacker ||
          move_maker.move_type() == MoveType::kPerpetualAttackee) {
        ++(stats->repetition_detected);
      }

      const Score current_alpha = std::max(result.score, alpha);

      SearchParams<debug_options> child_params;
      DebugModifyChildParams(node_id, move, &child_params);
      const auto child_result =
          Search(board, tt, OtherSide(side), depth - 1, -beta, -current_alpha,
                 stats, child_params);
      const Score current_score = -child_result.score;

      if (current_score > result.score) {
        result.score = current_score;
        result.best_move = move;
      }

      if (result.score >= beta) break;
    }
  }

  // Store TT.
  if (!tt_hit) StoreTT(board, tt, side, depth, alpha, beta, result);

  DebugLogCurrentNode(node_id, params, side, depth, alpha, beta, tt_hit,
                      result);
  return result;
}

}  // namespace

SearchResult Search(Board* board, TranspositionTable* tt, Side side,
                    int depth) {
  board->ResetRepetitionHistory();
  Stats stats;
#ifdef NDEBUG
  constexpr DebugOptions debug = kDebugOff;
#else
  constexpr DebugOptions debug = kDebugOn;
#endif
  SearchParams<debug> params;
  const auto result =
      Search(board, tt, side, depth, -kMateScore, kMateScore, &stats, params);
  stats.Print();
  return result;
}

void DebugPrintLogs() { Logger::GetInstance()->Print(); }

}  // namespace blur
