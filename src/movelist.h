#pragma once

#include "board.h"

class MoveList
{
    public:
        MoveList(Board *board, int side, MOVE first_move, int *history_scores);

        MOVE next_move();

    private:
        Board *board;
        int side;
        MOVE first_move;

        enum STATE
        {
            FIRST_MOVE,
            GENERATE_CAPTURES,
            GOOD_CAPTURES,
            GENERATE_MOVES,
            OTHERS
        };
        STATE state;

        MOVE moves[120];
        int scores[120], *history_scores;
        int c, moves_count;
};
