#pragma once

#include "board.h"

class Agent
{
    public:
        int search(Board &board, int side, Move *result);

        static const int INF = 0xfffffff;

    protected:
        int alpha_beta(Board &board, int side, Move *result, int depth, int alpha, int beta);
};
