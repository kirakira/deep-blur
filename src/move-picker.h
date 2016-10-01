#ifndef BLUR_MOVE_PICKER_H
#define BLUR_MOVE_PICKER_H

#include "board.h"

namespace blur {

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

    enum class Stage {
      kTTMove,
      kRegularMoves,
      kDone,
    };

    Iterator(const MovePicker& picker);
    Iterator(const MovePicker& picker, Stage begin_stage);

    void SkipInvalidMoves();

    const MovePicker* picker_ = nullptr;
    Stage stage_ = Stage::kTTMove;
    MoveList moves_;
    const Move* current_move_ = nullptr;
  };

  // tt_move can be invalid or corropted.
  MovePicker(const Board& board, Side side, Move tt_move)
      : board_(board), side_(side), tt_move_(tt_move) {}

  Iterator begin();
  Iterator end();

 private:
  const Board& board_;
  const Side side_;
  const Move tt_move_;
};

}  // namespace blur

#endif  // BLUR_MOVE_PICKER_H
