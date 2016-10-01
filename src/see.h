#ifndef BLUR_SEE_H
#define BLUR_SEE_H

#include "board.h"

namespace blur {

// Requires: capture must be a valid capture.
Score StaticExchangeEvaluation(Board* board, Move capture);

}  // namespace blur

#endif  // BLUR_SEE_H
