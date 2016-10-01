#include "common.h"
#include "move-picker.h"

namespace blur {

MovePicker::Iterator::Iterator(const MovePicker& picker)
    : Iterator(picker, Stage::kTTMove) {}

MovePicker::Iterator::Iterator(const MovePicker& picker, Stage begin_stage)
    : picker_(&picker), stage_(begin_stage) {
  SkipInvalidMoves();
}

void MovePicker::Iterator::SkipInvalidMoves() {
  switch (stage_) {
    case Stage::kTTMove:
      if (picker_->tt_move_.IsValid()) break;
      stage_ = Stage::kRegularMoves;
    // Fall-through intended.

    case Stage::kRegularMoves:
      if (!current_move_) {
        moves_ = picker_->board_.GenerateMoves(picker_->side_);
        current_move_ = moves_.begin();
      }
      // Skip tt move.
      while (current_move_ != moves_.end() &&
             *current_move_ == picker_->tt_move_) {
        ++current_move_;
      }
      if (current_move_ != moves_.end()) break;
      stage_ = Stage::kDone;
    // Fall-through intended.

    case Stage::kDone:
      // No-op.
      break;
  }
}

Move MovePicker::Iterator::operator*() const {
  switch (stage_) {
    case Stage::kTTMove:
      return picker_->tt_move_;

    case Stage::kRegularMoves:
      return *current_move_;

    case Stage::kDone:
      Die("No more moves to pick.");
  }
}

MovePicker::Iterator& MovePicker::Iterator::operator++() {
  switch (stage_) {
    case Stage::kTTMove:
      stage_ = Stage::kRegularMoves;
      break;

    case Stage::kRegularMoves:
      ++current_move_;
      break;

    case Stage::kDone:
      Die("Cannot advance a picker already done.");
  }

  SkipInvalidMoves();
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
