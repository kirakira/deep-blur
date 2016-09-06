#ifndef BLUR_LOGGER_H
#define BLUR_LOGGER_H

#include <iostream>
#include <vector>

#include "board-base.h"
#include "common.h"
#include "eval.h"

namespace blur {

class Logger {
 public:
  struct SearchNodeInfo {
    SearchNodeInfo(int64 current_id, int64 parent, Move parent_move,
                   Side current_side, int current_depth, Score initial_alpha,
                   Score initial_beta)
        : id(current_id),
          parent_id(parent),
          from_move(parent_move),
          side(current_side),
          depth(current_depth),
          alpha(initial_alpha),
          beta(initial_beta) {}
    int64 id;
    int64 parent_id;
    Move from_move;
    Side side;
    int depth;
    Score alpha;
    Score beta;
    bool tt_hit;
    Score score;
    Move best_move;
  };

  void SaveNode(const SearchNodeInfo& info);

  void Print() const;

  static Logger* GetInstance() {
    static Logger* logger = new Logger();
    return logger;
  }

 private:
  Logger() = default;

  std::vector<SearchNodeInfo> nodes_;
};

}  // namespace blur

#endif  // BLUR_LOGGER_H
