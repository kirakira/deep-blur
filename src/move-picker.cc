#include "move-picker.h"
#include "common.h"

namespace blur {

void KillerStats::RecordBetaCut(int depth, Move move) {
  if (!killers_[depth][0].IsValid()) {
    killers_[depth][0] = move;
  } else if (killers_[depth][0] != move) {
    killers_[depth][1] = killers_[depth][0];
    killers_[depth][0] = move;
  }
}

Move KillerStats::GetKiller1(int depth) const { return killers_[depth][0]; }

Move KillerStats::GetKiller2(int depth) const { return killers_[depth][1]; }

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
      moves_buffer_.Add(picker_->killer_stats_->GetKiller1(picker_->depth_));
      moves_buffer_.Add(picker_->killer_stats_->GetKiller2(picker_->depth_));
      break;
    }

    case Stage::kRegularMoves: {
      moves_buffer_ = picker_->board_.GenerateMoves(picker_->side_);
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
