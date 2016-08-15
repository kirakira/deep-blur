#include "search.h"

#include <iostream>
#include <string>
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
    move_made_ = board->CheckedMake(side, move);
  }

  ~CheckedMoveMaker() {
    if (move_made_) board_->Unmake();
  }

  bool MoveMade() const { return move_made_; }

 private:
  Board* const board_;
  bool move_made_;
};

struct Stats {
  int64 nodes_visited = 0;
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
                         const int, const Score, const Score, SearchResult) {}

#ifndef NDEBUG
template <>
void DebugLogCurrentNode<kDebugOn>(int64 node_id,
                                   const SearchParams<kDebugOn> params,
                                   Side side, const int depth,
                                   const Score alpha, const Score beta,
                                   SearchResult result) {
  Logger::SearchNodeInfo log_node_info(node_id, params.debug_info.parent_id,
                                       params.debug_info.from_move, side, depth,
                                       alpha, beta);
  log_node_info.score = result.score;
  log_node_info.best_move = result.best_move;
  Logger::GetInstance()->SaveNode(log_node_info);
}
#endif

// Only scores within (alpha, beta) are exact. Scores <= alpha are upper bounds;
// scores >= beta are lower bounds.
template <DebugOptions debug_options>
SearchResult Search(Board* const board, const Side side, const int depth,
                    const Score alpha, const Score beta, Stats* const stats,
                    const SearchParams<debug_options> params) {
  const int64 node_id = (stats->nodes_visited)++;
  SearchResult result;

  if (depth == 0) {
    Score score = board->Evaluation();
    result.score = (side == Side::kRed ? score : -score);
  } else {
    result.score = -kMateScore;
    for (const auto move : board->GenerateMoves(side)) {
      CheckedMoveMaker move_maker(board, side, move);
      if (!move_maker.MoveMade()) continue;

      const Score current_alpha = std::max(result.score, alpha);

      SearchParams<debug_options> child_params;
      DebugModifyChildParams(node_id, move, &child_params);
      const auto child_result = Search(board, OtherSide(side), depth - 1, -beta,
                                       -current_alpha, stats, child_params);
      const Score current_score = -child_result.score;

      if (current_score > result.score) {
        result.score = current_score;
        result.best_move = move;
      }

      if (result.score >= beta) break;
    }
  }

  DebugLogCurrentNode(node_id, params, side, depth, alpha, beta, result);
  return result;
}

}  // namespace

SearchResult Search(Board* board, Side side, int depth) {
  Stats stats;
#ifdef NDEBUG
  constexpr DebugOptions debug = kDebugOff;
#else
  constexpr DebugOptions debug = kDebugOn;
#endif
  SearchParams<debug> params;
  return Search(board, side, depth, -kMateScore, kMateScore, &stats, params);
}

void DebugPrintLogs() { Logger::GetInstance()->Print(); }

}  // namespace blur
