#pragma once

#include <ctime>

#include "common.h"
#include "board.h"
#include "transposition.h"

//#define DEBUG_OUTPUT

class Agent
{
    public:
        Agent();
        int search(Board &board, int side, MOVE *result, int time_limit, int depth);
        int quiescence(Board &board, int side, int alpha, int beta, POSITION last_square = INVALID_POSITION);

    protected:
        static const bool USE_NULL_MOVE = true;
        static const bool USE_TRANS_TABLE = true;
        static const bool USE_IID = true;
        static const bool USE_KILLER = true;

        static const bool USE_LMR = true;
        static const int LMR_NODES = 2, LMR_DEPTH = 3;

        static const bool CHECKS_IN_QUIESCENCE = false;
        static const int CHECK_TIME_NODES = 32768;

        static const int ABORTED = -INF - 1;

        int select_best_move(int *scores, int moves_count);
        void order_moves(MOVE *moves, int *scores, int moves_count, int order_count);

        int id(Board &board, int side, MOVE *result, clock_t deadline, int *depth);
        int search_root(Board &board, int side, MOVE *result, int depth, clock_t deadline,
                MOVE first_move, PV *pv, bool *aborted);
        int alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, int ply,
                clock_t deadline, bool nullable, POSITION last_square, bool isPV, PV *pv, bool *store_tt);

        int quiescence(Board &board, int side, int alpha, int beta, bool in_check, POSITION last_square, bool *store_tt);

        int trans_hit, nodes, null_cut, first_cut, beta_nodes, alpha_nodes, first_best;
        int move_score[1 << 16];
        int killer[MAX_DEPTH][2];

        double ebf(int nodes, int depth);

        bool special_move_type(MoveType mt, int *score, bool *store_tt);

        Transposition trans;

        clock_t search_start_time;
        void output_thinking(int ply, int score, PV *pv);

        void update_history(int depth, MOVE best_move, MOVE *searched_moves, int count);
};
