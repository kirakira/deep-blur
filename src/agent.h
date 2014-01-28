#pragma once

#include "board.h"

class Agent
{
    public:
        Agent();
        int search(Board &board, int side, MOVE *result);

        static const int INF = 0xfffffff;

    protected:
        int alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable);
        class MoveComparator
        {
            int *score_table;
            public:
                MoveComparator(int *table);

                bool operator()(const MOVE &x, const MOVE &y) const;
        };
        MoveComparator move_comparator;

        int firstHit, secondHit, miss;
        int move_score[1 << 16];
};
