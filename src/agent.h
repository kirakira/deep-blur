#pragma once

#include "board.h"
#include "transposition.h"

class Agent
{
    public:
        Agent();
        int search(Board &board, int side, MOVE *result, int depth);

        static const int INF = 2047;

    protected:
        int alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable);
        int id(Board &board, int side, MOVE *result, int depth);

        class MoveComparator
        {
            int *score_table;
            MOVE transp_move;
            POSITION king_pos;

            public:
                bool operator()(const MOVE &x, const MOVE &y) const;
                void set(int *table);
                void set(MOVE trans_move, POSITION king_p);
        };
        MoveComparator move_comparator;

        int firstHit, secondHit, miss;
        int trans_hit, nodes;
        int move_score[1 << 16];

        Transposition trans;
};
