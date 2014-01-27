#pragma once

#include "board.h"

class Agent
{
    public:
        int search(Board &board, int side, MOVE *result);

        static const int INF = 0xfffffff;

    protected:
        int alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable);

        int firstHit, secondHit, miss;
        int move_score[65535];
};
