#ifndef BLUR_MOVE_PICKER_H
#define BLUR_MOVE_PICKER_H

#include <limits>

#include "board.h"

namespace blur {

class KillerStats {
 public:
  inline void RecordBetaCut(int depth, Move move) {
    if (killers_[depth][0] != move) {
      killers_[depth][1] = killers_[depth][0];
      killers_[depth][0] = move;
    }
  }
  inline Move GetKiller1(int depth) const { return killers_[depth][0]; }
  inline Move GetKiller2(int depth) const { return killers_[depth][1]; }

 private:
  Move killers_[kMaxDepth][2];
};

struct HistoryMoveStats {
 public:
  HistoryMoveStats() : scores_() {}

  inline void RecordBestMove(Side side, Move move, int depth) {
    int& value =
        scores_[static_cast<int>(side)][move.from().value()][move.to().value()];
    value += depth * depth + 2 * depth - 2;
    if (value > kMaxMoveScore) value = kMaxMoveScore;
  }

  inline int GetMoveScore(Side side, Move move) const {
    return scores_[static_cast<int>(side)][move.from().value()]
                  [move.to().value()];
  }

 private:
  // Scores indexed by [side][source][dest].
  int scores_[2][kNumPositions][kNumPositions];
  static constexpr int kMaxMoveScore = std::numeric_limits<int>::max() / 2;
};

class MovePicker {
 public:
  class Iterator {
   public:
    // An uninitialized iterator.
    Iterator() = default;

    // Returned move could be invalid but uncorropted.
    Move operator*() const;
    Iterator& operator++();
    friend bool operator==(const Iterator& i1, const Iterator& i2);
    friend bool operator!=(const Iterator& i1, const Iterator& i2);

   private:
    friend class MovePicker;

    enum class Stage : int {
      kTTMove = 0,
      kCaptures,
      kKillers,
      kRegularMoves,
      kDone,
    };

    Iterator(const MovePicker& picker);
    Iterator(const MovePicker& picker, Stage begin_stage);

    // Fill moves_buffer_ for current stage and set current_move_ to its
    // begin().
    void PrepareMovesForCurrentStage();
    // Skip all moves that have been returned, advancing current stage if
    // needed.
    // Post condition: current_move_ points to a new move, or stage_ is kDone.
    void SkipOldMoves();
    static Stage NextStage(Stage stage);

    const MovePicker* picker_ = nullptr;
    Stage stage_ = Stage::kTTMove;
    MoveList moves_buffer_;
    MoveList moves_returned_;
    const Move* current_move_ = nullptr;
  };

  // tt_move can be invalid or corropted.
  MovePicker(const Board& board, Side side, Move tt_move, Move killer1,
             Move killer2, const HistoryMoveStats* history_stats)
      : board_(board),
        side_(side),
        tt_move_(tt_move),
        killer1_(killer1),
        killer2_(killer2),
        history_stats_(*history_stats) {}

  Iterator begin();
  Iterator end();

 private:
  const Board& board_;
  const Side side_;
  const Move tt_move_;
  const Move killer1_, killer2_;
  const HistoryMoveStats& history_stats_;
};

}  // namespace blur

#endif  // BLUR_MOVE_PICKER_H
