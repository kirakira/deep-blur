#include "common.h"
#include "eval.h"
#include "piece-position-eval.h"
#include "piece-value-eval.h"

namespace blur {

std::unique_ptr<Evaluator> Evaluator::Make(const std::string &name) {
  if (name == "piece-position") {
    return std::make_unique<PiecePositionEvaluator>();
  } else if (name == "piece-value") {
    return std::make_unique<PieceValueEvaluator>();
  } else {
    CHECK(false);
  }
}

} // namespace blur
