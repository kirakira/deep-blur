#include "search.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

#include "logger.h"
#include "move-picker.h"

using std::string;
using std::vector;

namespace blur {

namespace {

constexpr int64 kCheckTimeNodes = 1LL << 17;

// A scoped move maker.
class ScopedMoveMaker {
 public:
  ScopedMoveMaker(Board* board, Move move) : board_(board) {
    move_type_ = board->Make(move);
  }

  ~ScopedMoveMaker() { board_->Unmake(); }

  MoveType move_type() const { return move_type_; }

 private:
  Board* const board_;
  MoveType move_type_;
};

std::string ToHumanReadableScale(double x) {
  if (x >= 1000000000) {
    return std::to_string(x / 1000000000) + "G";
  } else if (x >= 1000000) {
    return std::to_string(x / 1000000) + "M";
  } else if (x >= 1000) {
    return std::to_string(x / 1000) + "K";
  } else {
    return std::to_string(x);
  }
}

struct Stats {
  int64 nodes_visited = 0;
  int64 nodes_expanded = 0;
  int64 tt_hit = 0;
  int64 affected_by_history = 0;
  static constexpr int kBestMoveRankSize = 10;
  int64 best_move_index[kBestMoveRankSize] = {0};
  int64 best_move_total = 0;

  void IncrementBestMoveRank(int rank) {
    ++best_move_total;
    if (rank < kBestMoveRankSize) ++best_move_index[rank];
  }

  void Print(const Timer &timer) {
    double elapsed_seconds =
        std::chrono::duration<double>(timer.GetReading()).count();
    std::cout << "# time " << elapsed_seconds << "s, visited " << nodes_visited
              << " ("
              << ToHumanReadableScale(static_cast<double>(nodes_visited) /
                                      elapsed_seconds)
              << " NPS), expanded " << nodes_expanded << " ("
              << ToHumanReadableScale(static_cast<double>(nodes_expanded) /
                                      elapsed_seconds)
              << " NPS)" << std::endl;
    std::cout << "# tt hit: "
              << static_cast<double>(tt_hit) * 100 /
                     static_cast<double>(nodes_visited)
              << "%, affected by history: "
              << static_cast<double>(affected_by_history) * 100 /
                     static_cast<double>(nodes_visited)
              << "%" << std::endl;
    std::cout << "# best move ranks: ";
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
  // Number of moves made since root position.
  int ply = 0;
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
SearchParams<debug_options>
BuildSearchParamsForChild(const SearchParams<debug_options> &parent_params,
                          int64 node_id, Move from_move,
                          int ply_increment = 1) {
  SearchParams<debug_options> child_params = parent_params;
  child_params.ply += ply_increment;
  DebugModifyChildParams(node_id, from_move, &child_params);
  return child_params;
}

// TODO: rename this to PerThreadSearchHelpers.
struct SearchSharedObjects {
  TranspositionTable* tt;
  Timer* timer;
  KillerStats killer_stats;
  Stats stats;
  HistoryMoveStats history_move_stats;
};

enum class PVType : int {
  kPV,
  kNonPV,
};

enum class RootType : int {
  kRoot,
  kNonRoot,
};

enum class NullMoveType : int {
  kNullable,
  kNonNullable,
};

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
  bool aborted = false;
  bool affected_by_history = false;
  // Only populated in PV nodes. Stored in reverse order. May be incomplete if
  // a checkmate is detected or if the result was a consequence of a repetition.
  std::vector<Move> pv;
};

Move CheckMoveOrEmpty(Board* board, Side side, Move move) {
  return board->IsPseudoLegalMove(side, move) ? move : Move();
}

// If it returns true, the move in result must be a valid move not causing
// repetition or perpetual attacks; if it returns false, tt_move could be
// an empty move or a legal move.
template <NullMoveType null_move_type>
bool ProbeTT(Board *board, TranspositionTable *tt, Side side, int depth,
             Score alpha, Score beta, InternalSearchResult *result,
             Move *tt_move) {
  *tt_move = Move();
  const uint64 hash = board->HashCode(side);
  TTEntry entry;
  if (!tt->LookUp(hash, &entry)) return false;
  if (entry.depth >= depth &&
      (entry.type == ScoreType::kExact ||
       (entry.type == ScoreType::kUpperBound && entry.score <= alpha) ||
       (entry.type == ScoreType::kLowerBound && entry.score >= beta))) {
    // Check if the move is valid.
    if (entry.best_move == kNullMove &&
        null_move_type == NullMoveType::kNonNullable) {
      return false;
    }
    if (depth >= 1 && entry.score != -Score::kMateScore &&
        entry.best_move != kNullMove) {
      if (!entry.best_move.IsValid() ||
          !board->IsPseudoLegalMove(side, entry.best_move)) {
        return false;
      }
      ScopedMoveMaker move_maker(board, entry.best_move);
      if (move_maker.move_type() == MoveType::kPerpetualAttacker ||
          move_maker.move_type() == MoveType::kRepetition ||
          move_maker.move_type() == MoveType::kPerpetualAttackee) {
        // We cannot use the score if it will cause a repetition, but the move
        // can be used to inspire search.
        *tt_move = entry.best_move;
        return false;
      }
    }
    result->external_result.score = entry.score;
    result->external_result.best_move = entry.best_move;
    result->affected_by_history = false;
    return true;
  }
  // If the tt result is unusable, we still populate tt_move to inspire move
  // ordering in search.
  *tt_move = CheckMoveOrEmpty(board, side, entry.best_move);
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

enum class ThinkingType {
  kNewPV,
  kEndOfIteration,
};

void OutputThinking(int depth, ThinkingType thinking_type, Score score,
                    const Timer& timer, int64 num_nodes,
                    const std::vector<Move>& pv) {
  using CentiSeconds = std::chrono::duration<int64, std::centi>;
  string type_string;
  if (thinking_type == ThinkingType::kEndOfIteration) type_string = ".";
  std::cout
      << depth << type_string << "\t" << score << "\t"
      << std::chrono::duration_cast<CentiSeconds>(timer.GetReading()).count()
      << "\t" << num_nodes << "\t";
  for (auto i = pv.rbegin(); i != pv.rend(); ++i) {
    std::cout << i->ToString() << " ";
  }
  std::cout << std::endl;
}

struct QuiescenceResult {
  Score score = -kMateScore;
  bool affected_by_history = false;
};

QuiescenceResult QuiescenceInternal(Board* board, TranspositionTable* tt,
                                    Side side, const Score alpha,
                                    const Score beta, bool in_check,
                                    SearchSharedObjects* shared_objects) {
  QuiescenceResult result;

  Move tt_move;
  if (tt) {
    InternalSearchResult search_result;
    if (ProbeTT<NullMoveType::kNullable>(board, tt, side, /*depth=*/0, alpha,
                                         beta, &search_result, &tt_move)) {
      result.score = search_result.external_result.score;
      return result;
    }
  }

  if (in_check) {
    result.score = -kMateScore;
  } else {
    const Score eval = board->Evaluation();
    result.score = side == Side::kRed ? eval : -eval;
  }

  if (result.score < beta) {
    for (const auto move :
         MovePicker(*board, side, tt_move, Move() /* killer1 */,
                    Move() /* killer2 */, &shared_objects->history_move_stats,
                    !in_check /* captures_only */)) {
      ScopedMoveMaker move_maker(board, move);
      if (board->InCheck(side)) continue;

      QuiescenceResult child_result;
      if (move_maker.move_type() == MoveType::kKingCapture) {
        child_result.score = -kMateScore;
      } else if (move_maker.move_type() != MoveType::kRegular &&
                 move_maker.move_type() != MoveType::kCapture) {
        child_result.score = -ScoreFromRepetitionRule(move_maker.move_type());
        child_result.affected_by_history = true;
      } else {
        const bool next_in_check = board->InCheck(OtherSide(side));
        if (in_check || next_in_check ||
            move_maker.move_type() == MoveType::kCapture) {
          child_result = QuiescenceInternal(board, tt, OtherSide(side), -beta,
                                            -std::max(alpha, result.score),
                                            next_in_check, shared_objects);
        }
      }

      result.score = std::max(result.score, -child_result.score);

      if (child_result.affected_by_history) result.affected_by_history = true;

      if (result.score >= beta) {
        result.affected_by_history = child_result.affected_by_history;
        break;
      }
    }
  }

  if (tt && !result.affected_by_history) {
    SearchResult result_for_tt;
    result_for_tt.score = result.score;
    StoreTT(board, tt, side, 0 /* depth */, alpha, beta, result_for_tt);
  }

  return result;
}

QuiescenceResult Quiescence(Board* board, TranspositionTable* tt, Side side,
                            const Score alpha, const Score beta,
                            SearchSharedObjects* shared_objects) {
  return QuiescenceInternal(board, tt, side, alpha, beta, board->InCheck(side),
                            shared_objects);
}

template <PVType node_type, RootType root_type, NullMoveType null_move_type,
          DebugOptions debug_options>
InternalSearchResult Search(const SearchOptions &options, Board *const board,
                            const Side side, const int depth, const Score alpha,
                            const Score beta,
                            const SearchParams<debug_options> params,
                            SearchSharedObjects *shared_objects);

template <PVType node_type, RootType root_type, DebugOptions debug_options>
InternalSearchResult
ExpandNode(const SearchOptions &options, Board *const board, const Side side,
           const int depth, const Score alpha, const Score beta, int64 node_id,
           Move tt_move, const SearchParams<debug_options> params,
           SearchSharedObjects *shared_objects) {
  ++shared_objects->stats.nodes_expanded;

  InternalSearchResult result;

  int best_move_index = -1;
  int num_moves_tried = 0;
  for (const auto move : MovePicker(
           *board, side, tt_move,
           CheckMoveOrEmpty(
               board, side,
               shared_objects->killer_stats.GetKiller1(params.ply)),
           CheckMoveOrEmpty(
               board, side,
               shared_objects->killer_stats.GetKiller2(params.ply)),
           &shared_objects->history_move_stats, false /* captures_only */)) {
    ScopedMoveMaker move_maker(board, move);
    if (board->InCheck(side)) continue;

    ++num_moves_tried;

    const Score current_alpha = std::max(result.external_result.score, alpha);

    InternalSearchResult child_result;
    if (move_maker.move_type() == MoveType::kKingCapture) {
      child_result.external_result.score = -kMateScore;
    } else if (move_maker.move_type() != MoveType::kRegular &&
               move_maker.move_type() != MoveType::kCapture) {
      child_result.external_result.score =
          -ScoreFromRepetitionRule(move_maker.move_type());
      // We won't have a best_move in this case.
      child_result.affected_by_history = true;
    } else {
      const SearchParams<debug_options> child_params =
          BuildSearchParamsForChild(params, node_id, move);

      bool do_full_window_search = false;
      if (node_type == PVType::kNonPV || num_moves_tried > 1) {
        // Use a null-window at non-pv nodes and expected cut moves at pv
        // nodes.
        child_result =
            Search<PVType::kNonPV, RootType::kNonRoot, NullMoveType::kNullable>(
                options, board, OtherSide(side), depth - 1,
                -(current_alpha + 1), -current_alpha, child_params,
                shared_objects);
        // Do a re-search with full window if the score fails high at pv node.
        do_full_window_search =
            !child_result.aborted && node_type == PVType::kPV &&
            -child_result.external_result.score > result.external_result.score;
      } else {
        do_full_window_search = true;
      }

      if (do_full_window_search) {
        child_result =
            Search<PVType::kPV, RootType::kNonRoot, NullMoveType::kNullable>(
                options, board, OtherSide(side), depth - 1, -beta,
                -current_alpha, child_params, shared_objects);
      }
    }

    // Abort if we reach the time limit.
    if (child_result.aborted) {
      result.aborted = true;
      break;
    }

    const Score current_score = -child_result.external_result.score;

    if (current_score > result.external_result.score) {
      result.external_result.score = current_score;
      result.external_result.best_move = move;
      best_move_index = num_moves_tried - 1;
      if (node_type == PVType::kPV) {
        child_result.pv.reserve(120);
        child_result.pv.push_back(move);
        result.pv = std::move(child_result.pv);

        if (root_type == RootType::kRoot) {
          OutputThinking(depth, ThinkingType::kNewPV, current_score,
                         *shared_objects->timer,
                         shared_objects->stats.nodes_visited, result.pv);
        }
      }
    }
    // Set affected_by_history as long as there's one child affected by
    // history.
    if (child_result.affected_by_history) {
      result.affected_by_history = true;
    }

    if (result.external_result.score >= beta) {
      shared_objects->killer_stats.RecordBetaCut(params.ply, move);
      // Reset affected_by_history if a beta-cutoff happens.
      result.affected_by_history = child_result.affected_by_history;
      break;
    }
  }

  // Update history move table and stats.
  if (!result.aborted && result.external_result.best_move.IsValid()) {
    shared_objects->history_move_stats.RecordBestMove(
        side, result.external_result.best_move, depth);
    shared_objects->stats.IncrementBestMoveRank(best_move_index);
  }

  return result;
}

// Only scores within (alpha, beta) are exact. Scores <= alpha are upper bounds;
// scores >= beta are lower bounds.
template <PVType node_type, RootType root_type, NullMoveType null_move_type,
          DebugOptions debug_options>
InternalSearchResult Search(const SearchOptions &options, Board *const board,
                            const Side side, const int depth, const Score alpha,
                            const Score beta,
                            const SearchParams<debug_options> params,
                            SearchSharedObjects *shared_objects) {
  const int64 node_id = shared_objects->stats.nodes_visited++;
  InternalSearchResult result;

  if (node_id % kCheckTimeNodes == 0 &&
      shared_objects->timer->GetReading() > options.time_limit) {
    result.aborted = true;
    return result;
  }

  // Probe tt.
  bool tt_hit = false;
  Move tt_move;
  const bool use_tt = options.use_tt && depth >= 1;
  constexpr bool effective_nullable =
      node_type == PVType::kNonPV && null_move_type == NullMoveType::kNullable;
  if (use_tt && ProbeTT < effective_nullable
          ? NullMoveType::kNullable
          : NullMoveType::kNonNullable > (board, shared_objects->tt, side,
                                          depth, alpha, beta, &result,
                                          &tt_move) &&
                node_type != PVType::kPV) {
    ++shared_objects->stats.tt_hit;
    tt_hit = true;
  } else if (depth == 0) {
    if (options.enable_quiescence) {
      TranspositionTable *tt_quiescence = nullptr;
      if (options.use_tt_in_quiescence) {
        tt_quiescence = shared_objects->tt;
      }
      const QuiescenceResult quiescence_result =
          Quiescence(board, tt_quiescence, side, alpha, beta, shared_objects);
      result.external_result.score = quiescence_result.score;
      result.affected_by_history = quiescence_result.affected_by_history;
    } else {
      const Score eval = board->Evaluation();
      result.external_result.score = side == Side::kRed ? eval : -eval;
    }
  } else {
    // Try null move.
    if (options.null_move_depth_reduction > 0 && effective_nullable) {
      const auto null_move_result = Search<PVType::kNonPV, RootType::kNonRoot,
                                           NullMoveType::kNonNullable>(
          options, board, OtherSide(side),
          std::max(0, depth - options.null_move_depth_reduction), -beta,
          -beta + 1, BuildSearchParamsForChild(params, node_id, kNullMove, 0),
          shared_objects);
      if (null_move_result.aborted) {
        result.aborted = true;
      } else if (-null_move_result.external_result.score >= beta) {
        result.external_result.score = -null_move_result.external_result.score;
        result.external_result.best_move = kNullMove;
        result.affected_by_history = null_move_result.affected_by_history;
      }
    }

    // Expand the node fully if the null move doesn't result in a cut.
    if (!result.aborted && result.external_result.score < beta) {
      result = ExpandNode<node_type, root_type>(options, board, side, depth,
                                                alpha, beta, node_id, tt_move,
                                                params, shared_objects);
    }
  }

  if (!result.aborted) {
    // Store TT.
    if (use_tt && !tt_hit &&
        result.external_result.best_move.IsValidOrNullMove()) {
      // If there are no more than 4 moves made, the result cannot be affected
      // by history.
      if (params.ply < 4) {
        result.affected_by_history = false;
      }
      if (result.affected_by_history) {
        ++shared_objects->stats.affected_by_history;
      } else if (depth > 0) {
        StoreTT(board, shared_objects->tt, side, depth, alpha, beta,
                result.external_result);
      }
    }

    DebugLogCurrentNode(node_id, params, side, depth, alpha, beta, tt_hit,
                        result.external_result);
  }
  return result;
}

SearchResult IterativeDeepening(Board* board, TranspositionTable* tt, Side side,
                                int depth, const SearchOptions& options) {
  CHECK(depth >= 1);
#ifdef NDEBUG
  constexpr DebugOptions debug = kDebugOff;
#else
  constexpr DebugOptions debug = kDebugOn;
#endif
  SearchSharedObjects shared_objects;
  shared_objects.tt = tt;
  Timer timer;
  shared_objects.timer = &timer;

  InternalSearchResult result;
  for (int d = 1; d <= depth; ++d) {
    SearchParams<debug> params;
    auto this_result =
        Search<PVType::kPV, RootType::kRoot, NullMoveType::kNonNullable>(
            options, board, side, d, -kMateScore, kMateScore, params,
            &shared_objects);
    // Override the previous result if this result is better, or this result is
    // not aborted.
    if (!this_result.aborted ||
        this_result.external_result.score >= result.external_result.score) {
      result = this_result;
    }

    if (this_result.aborted) break;
    OutputThinking(d, ThinkingType::kEndOfIteration,
                   result.external_result.score, timer,
                   shared_objects.stats.nodes_visited, result.pv);
  }
  shared_objects.stats.Print(timer);

  return result.external_result;
}

}  // namespace

const SearchOptions& SearchOptions::Defaults() {
  static auto* defaults = new SearchOptions;
  return *defaults;
}

SearchResult Search(Board* board, TranspositionTable* tt, Side side, int depth,
                    const SearchOptions& options) {
  // We assume the other side can do perpetual attacks in the search.
  board->ResetRepetitionHistory(OtherSide(side));
  return IterativeDeepening(board, tt, side, depth, options);
}

void DebugPrintLogs() { Logger::GetInstance()->Print(); }

}  // namespace blur
