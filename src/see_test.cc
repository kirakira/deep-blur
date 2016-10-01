#include "see.h"

using namespace blur;
using namespace std;

bool TestSEE() {
  Board board;
  if (!board.SetBoard("4k4/9/9/3h5/9/9/9/3R5/9/5K3")) return false;
  if (Score::kHorseScore != StaticExchangeEvaluation(&board, Move("d2d6"))) {
    return false;
  }
  if (!board.SetBoard("4k4/9/5h3/3h5/9/9/9/3R5/9/5K3")) return false;
  if (Score::kHorseScore - Score::kRookScore !=
      StaticExchangeEvaluation(&board, Move("d2d6"))) {
    return false;
  }
  if (!board.SetBoard("4k4/9/5h3/3r5/9/9/9/3R5/9/5K3")) return false;
  if (Score::kDrawScore != StaticExchangeEvaluation(&board, Move("d2d6"))) {
    return false;
  }
  if (!board.SetBoard("r3kR3/9/9/9/9/9/9/9/9/5K3")) return false;
  if (Score::kMateScore != StaticExchangeEvaluation(&board, Move("f9e9"))) {
    return false;
  }
  if (!board.SetBoard("2Rpk4/9/9/9/9/9/9/9/9/3K5")) return false;
  if (Score::kPawnScore != StaticExchangeEvaluation(&board, Move("c9d9"))) {
    return false;
  }
  if (!board.SetBoard("4k4/4h4/5h3/3c5/9/9/9/3R5/9/5K3")) return false;
  if (Score::kHorseScore - kRookScore !=
      StaticExchangeEvaluation(&board, Move("d2d6"))) {
    return false;
  }
  return true;
}

int main() {
  bool success = true;
  success = success && TestSEE();
  cout << (success ? "Success." : "Failed.") << endl;
  return success ? 0 : 1;
}
