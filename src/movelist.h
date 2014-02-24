#pragma once

#include "board.h"

class MoveList
{
    public:
        MoveList(Board *board, int side, MOVE first_move, int *history_scores, MOVE killer1, MOVE killer2);

        MOVE next_move();
        bool remaining_moves();

    private:
        Board *board;
        int side;
        MOVE first_move, killer1, killer2;

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

        static const int KILLER1_SCORE = 0x7fffffff, KILLER2_SCORE = 0x7ffffffe;
};
