#include "move-picker.h"
#include "common.h"

namespace blur {

MovePicker::Iterator::Iterator(const MovePicker& picker)
    : Iterator(picker, Stage::kTTMove) {}

MovePicker::Iterator::Iterator(const MovePicker& picker, Stage begin_stage)
    : picker_(&picker), stage_(begin_stage) {
  PrepareMovesForCurrentStage();
  SkipOldMoves();
}

/* static */
MovePicker::Iterator::Stage MovePicker::Iterator::NextStage(Stage stage) {
  return static_cast<Stage>(static_cast<int>(stage) + 1);
}

void MovePicker::Iterator::SkipOldMoves() {
  while (stage_ != Stage::kDone) {
    for (; current_move_ != moves_buffer_.end(); ++current_move_) {
      bool old_move = false;
      for (const auto move : moves_returned_) {
        if (*current_move_ == move) {
          old_move = true;
          break;
        }
      }
      if (!old_move) break;
    }
    if (current_move_ == moves_buffer_.end()) {
      stage_ = NextStage(stage_);
      PrepareMovesForCurrentStage();
    } else {
      break;
    }
  }
}

namespace {

void InsertionSort(Move* begin, Move* end, const HistoryMoveStats* stats,
                   Side side) {
  if (begin == end) return;
  for (Move* i = begin + 1; i != end; ++i) {
    const Move curr = *i;
    Move* j;
    for (j = i;
         j != begin &&
         stats->GetMoveScore(side, *(j - 1)) < stats->GetMoveScore(side, *j);
         --j) {
      *j = *(j - 1);
    }
    *j = curr;
  }
}

}  // namespace

void MovePicker::Iterator::PrepareMovesForCurrentStage() {
  switch (stage_) {
    case Stage::kTTMove: {
      moves_buffer_.Clear();
      if (picker_->tt_move_.IsValid()) {
        moves_buffer_.Add(picker_->tt_move_);
      }
      break;
    }

    case Stage::kCaptures: {
      moves_buffer_ = picker_->board_.GenerateCaptures(picker_->side_);
      break;
    }

    case Stage::kKillers: {
      moves_buffer_.Clear();
      moves_buffer_.Add(picker_->killer1_);
      moves_buffer_.Add(picker_->killer2_);
      break;
    }

    case Stage::kRegularMoves: {
      moves_buffer_ = picker_->board_.GenerateMoves(picker_->side_);
      InsertionSort(moves_buffer_.begin(), moves_buffer_.end(),
                    &picker_->history_stats_, picker_->side_);
      break;
    }

    case Stage::kDone: {
      moves_buffer_.Clear();
      break;
    }
  }
  current_move_ = moves_buffer_.begin();
}

Move MovePicker::Iterator::operator*() const { return *current_move_; }

MovePicker::Iterator& MovePicker::Iterator::operator++() {
  if (stage_ != Stage::kRegularMoves) moves_returned_.Add(*current_move_);
  ++current_move_;
  SkipOldMoves();
  return *this;
}

bool operator==(const MovePicker::Iterator& i1,
                const MovePicker::Iterator& i2) {
  return i1.stage_ == i2.stage_ &&
         i1.stage_ == MovePicker::Iterator::Stage::kDone;
}

bool operator!=(const MovePicker::Iterator& i1,
                const MovePicker::Iterator& i2) {
  return !(i1 == i2);
}

MovePicker::Iterator MovePicker::begin() { return Iterator(*this); }

MovePicker::Iterator MovePicker::end() {
  return Iterator(*this, Iterator::Stage::kDone);
}

}  // namespace blur
