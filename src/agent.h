#pragma once

#include "board.h"
#include "transposition.h"

class Agent
{
    public:
        Agent();
        int search(Board &board, int side, MOVE *result);

        static const int INF = 2047;

    protected:
        int alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable, int *move_score_table);
        int id(Board &board, int side, MOVE *result, int depth);

        class MoveComparator
        {
            int *score_table;
            public:
                bool operator()(const MOVE &x, const MOVE &y) const;
                void set(int *table);
        };
        MoveComparator move_comparator;

        int firstHit, secondHit, miss;
        int trans_hit, nodes;
        int move_score[2][1 << 16];

        Transposition trans;
};
