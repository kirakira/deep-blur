#include <vector>

#include "see.h"

using std::vector;

namespace blur {

namespace {

constexpr Score kPieceStaticValues[] = {
    Score::kDrawScore,      Score::kMateScore,     Score::kPawnScore,
    Score::kAssistantScore, Score::kElephantScore, Score::kHorseScore,
    Score::kCannonScore,    Score::kRookScore};

}  // namespace

Score StaticExchangeEvaluation(Board* board, Move capture) {
  vector<Score> values;

  int moves_made = 0;
  while (true) {
    values.push_back(kPieceStaticValues[static_cast<int>(
        board->PieceAt(capture.to()).type())]);
    if (board->PieceAt(capture.to()).type() == PieceType::kKing) break;

    board->Make(capture);
    ++moves_made;
    const auto pair = board->IsAttacked(capture.to());
    if (pair.first) {
      capture = Move(pair.second, capture.to());
    } else {
      break;
    }
  }
  while (moves_made-- > 0) board->Unmake();

  Score ans = Score::kDrawScore;
  while (!values.empty()) {
    if (values.size() == 1) {
      ans = values.back() - ans;
    } else {
      ans = std::max(Score::kDrawScore, values.back() - ans);
    }
    values.pop_back();
  }
  return ans;
}

}  // namespace blur
