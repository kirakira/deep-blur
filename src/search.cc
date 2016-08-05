#include "search.h"

#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace blur {

// A class that unmakes the move in destructor.
class CheckedMoveMaker {
 public:
  CheckedMoveMaker(Board& board, Side side, Move move) : board_(board) {
    move_made_ = board.CheckedMake(side, move);
  }

  ~CheckedMoveMaker() {
    if (move_made_) board_.Unmake();
  }

  bool MoveMade() const { return move_made_; }

 private:
  Board& board_;
  bool move_made_;
};

// Helper class to log the search tree.
class Logger {
 public:
  struct SearchNode {
    Move from_move;
    Side side;
    int depth;
    Score alpha;
    Score beta;
    vector<int> children;
    SearchResult result;
  };

  int NewSearch() {
    CHECK(stack_.empty());
    const int node = static_cast<int>(nodes_.size());
    stack_.push_back(node);
    nodes_.emplace_back();
    roots_.push_back(node);
    next_move_ = Move();
    return node;
  }

  void SetMove(Move move) {
    next_move_ = move;
  }

  void RecursionStart(const Board&, Side side, int depth, Score alpha,
                      Score beta) {
    CHECK(!stack_.empty());
    nodes_[stack_.back()].side = side;
    nodes_[stack_.back()].depth = depth;
    nodes_[stack_.back()].alpha = alpha;
    nodes_[stack_.back()].beta = beta;
  }

  void Recursion() {
    const int current_node = stack_.empty() ? -1 : stack_.back();

    const int new_node = static_cast<int>(nodes_.size());
    stack_.push_back(new_node);
    nodes_.emplace_back();
    nodes_.back().from_move = next_move_;

    if (current_node != -1) {
      nodes_[current_node].children.push_back(new_node);
    }

    next_move_ = Move();
  }

  SearchResult&& Return(SearchResult&& result) {
    CHECK(!stack_.empty());
    const int current_node = stack_.back();
    nodes_[current_node].result = result;
    stack_.pop_back();
    return std::move(result);
  }

  void Print() const {
    using std::cout;
    using std::endl;
    cout << "Roots: ";
    for (auto root : roots_) cout << root;
    cout << endl;
    for (int i = 0; i < static_cast<int>(nodes_.size()); ++i) {
      cout << "node " << i << " " << (nodes_[i].side == Side::kRed ? "r" : "b")
           << " depth " << nodes_[i].depth << " alpha " << nodes_[i].alpha
           << " beta " << nodes_[i].beta;
      cout << " children ";
      for (int c : nodes_[i].children) {
        cout << c << "(" << nodes_[c].from_move.ToString() << " "
             << -nodes_[c].result.score << ") ";
      }
      cout << "final " << nodes_[i].result.score << " "
           << nodes_[i].result.best_move.ToString() << endl;
    }
  }

  static Logger* GetInstance() {
    static Logger* logger = new Logger();
    return logger;
  }

 private:
  Logger() = default;

  vector<int> stack_;
  vector<SearchNode> nodes_;
  vector<int> roots_;
  Move next_move_;
};

#ifndef NDEBUG
#define LOG_NEW(call) \
  (Logger::GetInstance()->NewSearch(), Logger::GetInstance()->Return(call))
#define LOG_RECURSION_START(board, side, depth, alpha, beta) \
  (Logger::GetInstance()->RecursionStart(board, side, depth, alpha, beta))
#define LOG_MOVE(move) Logger::GetInstance()->SetMove(move)
#define LOG(call) \
  (Logger::GetInstance()->Recursion(), Logger::GetInstance()->Return(call))
#else
#define LOG_NEW(call) call
#define LOG_RECURSION_START(board, side, depth, alpha, beta)
#define LOG_MOVE(move)
#define LOG(call) call
#endif

// Only scores within (alpha, beta) are exact. Scores <= alpha are upper bounds;
// scores >= beta are lower bounds.
SearchResult Search(Board& board, Side side, int depth, Score alpha, Score beta) {
  LOG_RECURSION_START(board, side, depth, alpha, beta);
  SearchResult result;

  if (depth == 0) {
    Score score = board.Evaluation();
    result.score = (side == Side::kRed ? score : -score);
    return result;
  }

  result.score = -kMateScore;
  for (auto move : board.GenerateMoves(side)) {
    CheckedMoveMaker move_maker(board, side, move);
    if (!move_maker.MoveMade()) continue;
    LOG_MOVE(move);

    const Score current_alpha = std::max(result.score, alpha);
    const auto child_result =
        LOG(Search(board, OtherSide(side), depth - 1, -beta, -current_alpha));
    const Score current_score = -child_result.score;

    if (current_score > result.score) {
      result.score = current_score;
      result.best_move = move;
    }

    if (result.score >= beta) break;
  }

  return result;
}

SearchResult Search(Board& board, Side side, int depth) {
  return LOG_NEW(Search(board, side, depth, -kMateScore, kMateScore));
}

#undef LOG_NEW
#undef LOG_RECURSION_START
#undef LOG_MOVE
#undef LOG

void DebugPrintLogs() {
  Logger::GetInstance()->Print();
}

}  // namespace blur
