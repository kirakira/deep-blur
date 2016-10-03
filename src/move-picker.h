#ifndef BLUR_MOVE_PICKER_H
#define BLUR_MOVE_PICKER_H

#include "board.h"

namespace blur {

class KillerStats {
 public:
  void RecordBetaCut(int depth, Move move);
  Move GetKiller1(int depth) const;
  Move GetKiller2(int depth) const;

 private:
  Move killers_[kMaxDepth][2];
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
  MovePicker(const Board& board, Side side, Move tt_move, int depth,
             KillerStats* killer_stats)
      : board_(board),
        side_(side),
        tt_move_(tt_move),
        depth_(depth),
        killer_stats_(killer_stats) {}

  Iterator begin();
  Iterator end();

 private:
  const Board& board_;
  const Side side_;
  const Move tt_move_;
  const int depth_;
  KillerStats* const killer_stats_;
};

}  // namespace blur

#endif  // BLUR_MOVE_PICKER_H
