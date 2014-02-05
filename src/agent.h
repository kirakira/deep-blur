#pragma once

#include <vector>

#include "board.h"
#include "transposition.h"

class Agent
{
    public:
        Agent();
        int search(Board &board, int side, MOVE *result, int depth);
        int quiescence(Board &board, int side, int alpha, int beta);

        static const int INF = 2047;

    protected:
        int select_best_move(int *scores, int moves_count);
        void order_moves(MOVE *moves, int *scores, int moves_count, int order_count);
        int alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable);
        int id(Board &board, int side, MOVE *result, int depth);

        bool is_winning_capture(Board &board, MOVE move, int score, int side);
        int static_exchange_eval(Board &board, int side, POSITION pos);
        int quiescence(Board &board, int side, int alpha, int beta, std::vector<uint64_t> *rep, int *last_progress, bool in_check);

        int trans_hit, nodes, leaf, null_cut, first_cut, beta_cut, alpha_nodes, first_best;
        int move_score[1 << 16];

        Transposition trans;
};
