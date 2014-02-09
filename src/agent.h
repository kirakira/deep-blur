#pragma once

#include <vector>

#include "board.h"
#include "transposition.h"

//#define DEBUG_OUTPUT

class Agent
{
    public:
        Agent();
        int search(Board &board, int side, MOVE *result, int depth);
        int quiescence(Board &board, int side, int alpha, int beta, POSITION last_square = INVALID_POSITION);

        static const int INF = 2047;

    protected:
        static const int MAX_DEPTH = 80;
        static const int FULL_DEPTH_PLY = 3;
        static const bool USE_NULL_MOVE = true;
        static const bool USE_TRANS_TABLE = true;
        static const bool USE_IID = true;
        static const bool USE_KILLER = false;
        static const bool USE_LMR = false;
        static const int LMR_NODES = 3, LMR_PLY = 6;

        int select_best_move(int *scores, int moves_count);
        void order_moves(MOVE *moves, int *scores, int moves_count, int order_count);
        int alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, int ply,
                bool nullable, POSITION last_square = INVALID_POSITION);
        int id(Board &board, int side, MOVE *result, int depth);

        bool is_winning_capture(Board &board, MOVE move, int score, int side);
        int static_exchange_eval(Board &board, int side, POSITION pos);
        int quiescence(Board &board, int side, int alpha, int beta, std::vector<uint64_t> *rep, int *last_progress, bool in_check, POSITION last_square);

        int trans_hit, nodes, leaf, null_cut, first_cut, beta_nodes, alpha_nodes, first_best;
        int move_score[1 << 16];
        int killer[MAX_DEPTH][2];

        Transposition trans;
};
