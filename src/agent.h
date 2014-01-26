#pragma once

#include "board.h"

class Agent
{
    public:
        bool search(Board &board, int side, Move *result);


    protected:
        int alpha_beta(Board &board, int side, Move *result, int depth, int alpha, int beta);

        static const int INF = 0xfffffff;
};
