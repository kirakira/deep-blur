#include "piece-value-eval.h"

#include <array>

namespace blur {

namespace {

std::array<Score, 16> kPieceValues = []() {
  std::array<Score, 16> values{};
  for (Side side : {Side::kRed, Side::kBlack}) {
    int side_multiplier = (side == Side::kRed ? 1 : -1);
    values[Piece(side, PieceType::kAssistant).value()] =
        kAssistantScore * side_multiplier;
    values[Piece(side, PieceType::kElephant).value()] =
        kElephantScore * side_multiplier;
    values[Piece(side, PieceType::kHorse).value()] =
        kHorseScore * side_multiplier;
    values[Piece(side, PieceType::kCannon).value()] =
        kCannonScore * side_multiplier;
    values[Piece(side, PieceType::kPawn).value()] =
        kPawnScore * side_multiplier;
    values[Piece(side, PieceType::kRook).value()] =
        kRookScore * side_multiplier;
  }
  return values;
}();

}  // namespace

template <bool make>
Score UpdateScore(Piece captured_piece, Score score) {
  Score delta = kDrawScore;
  if (captured_piece != Piece::EmptyPiece()) {
    delta = kPieceValues[captured_piece.value()];
    if (!make) delta *= -1;
  }
  return score - delta;
}

void PieceValueEvaluator::OnMake(Move, Piece, Piece captured_piece) {
  score_ = UpdateScore<true>(captured_piece, score_);
}

void PieceValueEvaluator::OnUnmake(Move, Piece, Piece captured_piece) {
  score_ = UpdateScore<false>(captured_piece, score_);
}

void PieceValueEvaluator::SetBoard(const Piece board[]) {
  score_ = kDrawScore;
  for (int i = 0; i < kNumPositions; ++i) {
    Position pos(i);
    if (board[i] != Piece::EmptyPiece()) {
      score_ += kPieceValues[board[i].value()];
    }
  }
}

Score PieceValueEvaluator::CurrentScore() const { return score_; }

}  // namespace blur
