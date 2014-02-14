#pragma once

#include "board.h"

bool is_winning_capture(Board *board, MOVE move, int score, int side);

int static_exchange_eval(Board *board, int side, POSITION pos);
