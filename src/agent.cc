#include <iostream>
#include <cstring>
#include <ctime>
#include <cmath>

#include "agent.h"
#include "see.h"
#include "movelist.h"

using namespace std;

Agent::Agent()
    : trans(22)
{
}

int Agent::select_best_move(int *scores, int moves_count)
{
    int best = scores[0], besti = 0;
    for (int i = 1; i < moves_count; ++i)
        if (scores[i] > Board::NON_CAPTURE && scores[i] > best)
        {
            best = scores[i];
            besti = i;
        }
    return besti;
}

void Agent::order_moves(MOVE *moves, int *scores, int moves_count, int order_count)
{
    while (order_count > 0 && moves_count > 0)
    {
        int besti = select_best_move(scores, moves_count);
        if (besti != 0)
        {
            MOVE t = moves[besti];
            for (int i = besti; i > 0; --i)
                moves[i] = moves[i - 1];
            moves[0] = t;

            int tt = scores[besti];
            for (int i = besti; i > 0; --i)
                scores[i] = scores[i - 1];
            scores[0] = tt;
        }
        --moves_count;
        --order_count;
        ++moves;
        ++scores;
    }
}

double Agent::ebf(int nodes, int depth)
{
    double t = pow((double) nodes, 1.f / (double) depth);
    t = pow(((t - 1) * nodes / t + 1), 1.f / (double) depth);
    t = pow(((t - 1) * nodes / t + 1), 1.f / (double) depth);
    return t;
}

bool Agent::special_move_type(MoveType mt, int *score, bool *store_tt)
{
    if (mt == KING_CAPTURE)
    {
        *score = INF;
        *store_tt = true;
        return true;
    }
    else if (mt == PERPETUAL_CHECK_OR_CHASE)
    {
        *score = -INF;
        *store_tt = false;
        return true;
    }
    else if (mt == REPETITION)
    {
        *score = 0;
        *store_tt = false;
        return true;
    }
    else
        return false;
}

int Agent::search(Board &board, int side, MOVE *result, int time_limit, int depth)
{
    nodes = 0;

    search_start_time = clock();
    clock_t deadline = search_start_time + (double) time_limit / 1000. * (double) CLOCKS_PER_SEC;
    int ret = id(board, side, result, deadline, &depth);
    clock_t t = clock() - search_start_time;

    double sec = (double) t / (double) CLOCKS_PER_SEC;
    cout << "# total nodes: " << readable_number(nodes)
        << ", EBF: " << ebf(nodes, depth)
        << ", NPS: " << (double) nodes / sec / 1000000. << "m in " << sec << "s" << endl;
    cout << "# " << board.fen_string(side) << endl;

    return ret;
}

void Agent::output_thinking(int ply, int score, PV *pv)
{
    int t = (int) ((double) (clock() - search_start_time) / (double) CLOCKS_PER_SEC * 100);
    string sign;
    if (score > 0)
        sign = "+";
    cout << ply << "\t" << sign << score << "\t" << t << "\t" << nodes << "\t";
    if (pv)
    {
        for (int i = 0; i < pv->count; ++i)
            cout << move_string(pv->moves[i]) << " ";
    }
    cout << endl;
}

int Agent::id(Board &board, int side, MOVE *result, clock_t deadline, int *depth)
{
    int ret = 0;
    memset(move_score, 0, sizeof(move_score));
    memset(killer, 0, sizeof(killer));

    *result = 0;
    for (int level = 1; level <= *depth; ++level)
    {
        PV pv;
        pv.count = 0;

        bool aborted;
        MOVE current_move;

        int t = search_root(board, side, &current_move, level, deadline, *result, &pv, &aborted);
        if (t != ABORTED)
        {
            ret = t;
            *result = current_move;
        }

        if (aborted)
        {
            *depth = level - 1;
            break;
        }
    }
    return ret;
}

int Agent::search_root(Board &board, int side, MOVE *result, int depth, clock_t deadline,
        MOVE first_move, PV *pv, bool *aborted)
{
    uint64_t my_hash = board.hash_code(side);
    bool store_tt = true;

    MoveList ml(&board, side, first_move, move_score, 0, 0);

    *aborted = false;
    MOVE move, best_move = 0;
    int ans = ABORTED;

    for (int i = 0; ans < INF && (move = ml.next_move()); ++i)
    {
        MoveType mt;
        if (!board.move(move, &mt))
            continue;

        PV newPV;
        newPV.moves[0] = move;
        newPV.count = 1;

        int t;
        bool propagated_store;

        if (!special_move_type(mt, &t, &propagated_store))
        {
            POSITION dst = move_dst(move);

            if (i == 0)
                t = -alpha_beta(board, 1 - side, NULL, depth - 1, -INF, -ans,
                        1, deadline, true, dst, true, &newPV, &propagated_store);
            else
            {
                t = -alpha_beta(board, 1 - side, NULL, depth - 1, -ans - 1,
                        -ans, 1, deadline, true, dst, false, NULL, &propagated_store);
                if (ans < t)
                    t = -alpha_beta(board, 1 - side, NULL, depth - 1, -INF, -ans,
                            1, deadline, true, dst, true, &newPV, &propagated_store);
            }
        }
        board.unmove();

        if (t == -ABORTED)
        {
            *aborted = true;
            break;
        }

        if (t > ans)
        {
            best_move = move;
            ans = t;
            store_tt = propagated_store;
            if (pv)
            {
                pv->count = 0;
                catPV(pv, &newPV);
            }

            output_thinking(depth, ans, pv);
        }
    }

    if (USE_TRANS_TABLE && store_tt)
    {
        int e;
        if (*aborted)
            e = Transposition::LOWER;
        else
            e = Transposition::EXACT;
        trans.put(my_hash, ans, e, best_move, depth);
    }

    if (result)
        *result = best_move;
    return ans;
}

// if return value >= beta, it is a lower bound; if return value <= alpha, it is an upper bound
int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta,
        int ply, clock_t deadline, bool nullable, POSITION last_square,
        bool isPV, PV *pv, bool *store_tt)
{
    if (depth == 0)
    {
        ++nodes;
        return quiescence(board, side, alpha, beta, board.in_check(side), last_square, store_tt);
    }

    uint64_t my_hash = board.hash_code(side);

    *store_tt = true;

    int his_score, his_exact = 0, his_depth = 0;
    MOVE his_move = 0;
    MoveType mt;
    if (USE_TRANS_TABLE && trans.get(board.hash_code(side), &his_score, &his_exact, &his_move, &his_depth)
            && (his_move == 0 || board.checked_move(side, his_move, &mt)))
    {
        if (his_move != 0)
            board.unmove();

        if (his_depth >= depth && (his_move == 0 || mt != PERPETUAL_CHECK_OR_CHASE) &&
                (his_exact == Transposition::EXACT || (his_exact == Transposition::UPPER && his_score <= alpha)
                 || (his_exact == Transposition::LOWER && his_score >= beta))
                && (nullable || his_move != 0))
        {
            if (result)
                *result = his_move;
            return his_score;
        }

        if (his_exact == Transposition::UPPER && his_score <= beta - 1)
            nullable = false;
    }

    if (nodes % CHECK_TIME_NODES == 0 && clock() >= deadline)
        return ABORTED;

    ++nodes;

    int ans = -INF;
    MOVE best_move = 0;
    MOVE searched_moves[120];
    int searched_moves_count = 0;
    bool aborted = false, propagated_store;
    int eval = board.static_value(side);

    if (USE_NULL_MOVE && nullable && !isPV/* && eval >= beta*/)
    {
        int R = 3;
        ans = -alpha_beta(board, 1 - side, NULL, max(0, depth - R),
                -beta, -beta + 1, ply, deadline, false,
                INVALID_POSITION, isPV, NULL, &propagated_store);
        if (ans >= beta)
            *store_tt = propagated_store;
    }

    if (ans == -ABORTED)
        aborted = true;

    if (!aborted && ans < beta)
    {
        ans = -INF;

        if (USE_IID && depth >= 6)
            alpha_beta(board, side, &his_move, depth - 2, alpha, beta, ply + 1, deadline,
                    false, last_square, isPV, NULL, &propagated_store);

        int original_pv_count = pv ? pv->count : 0;
        MoveList ml(&board, side, his_move, move_score, killer[ply][0], killer[ply][1]);
        MOVE move;
        for (int i = 0; ans < beta && (move = ml.next_move()); ++i)
        {
            bool is_good_capture = board.is_capture(move) && is_winning_capture(&board, move, side);

            MoveType mt;
            if (!board.move(move, &mt))
                continue;

            PV newPV;
            newPV.moves[0] = move;
            newPV.count = 1;

            searched_moves[searched_moves_count++] = move;

            int t;
            if (!special_move_type(mt, &t, &propagated_store))
            {
                int current_alpha = max(alpha, ans);
                POSITION dst = move_dst(move);

                if (i == 0)
                    t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha,
                            ply + 1, deadline, true, dst, isPV, &newPV, &propagated_store);
                else
                {
                    t = current_alpha + 1;

                    if (USE_LMR && !isPV && depth > LMR_DEPTH && i >= LMR_NODES
                            && ml.remaining_moves() && !is_good_capture)
                    {
                        t = -alpha_beta(board, 1 - side, NULL, depth - 2, -current_alpha - 1,
                                -current_alpha, ply + 1, deadline, true, dst, false, NULL, &propagated_store);
                    }

                    if (t > current_alpha)
                        t = -alpha_beta(board, 1 - side, NULL, depth - 1, -current_alpha - 1,
                                -current_alpha, ply + 1, deadline, true, dst, false, NULL, &propagated_store);
                    if (current_alpha < t && t < beta)
                        t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha,
                                ply + 1, deadline, true, dst, isPV, &newPV, &propagated_store);
                }
            }

            board.unmove();

            if (t == -ABORTED)
            {
                aborted = true;
                break;
            }

            if (t > ans)
            {
                ans = t;
                *store_tt = propagated_store;
                best_move = move;
                if (isPV && pv)
                {
                    pv->count = original_pv_count;
                    catPV(pv, &newPV);
                }
            }

            if (t >= beta)
            {
                if (USE_KILLER && killer[ply][0] != move)
                {
                    killer[ply][1] = killer[ply][0];
                    killer[ply][0] = move;
                }
                break;
            }
        }
    }

    if (aborted)
        return ABORTED;

    if (USE_TRANS_TABLE && *store_tt
            && (his_exact != Transposition::EXACT || his_depth <= depth))
    {
        int e = Transposition::EXACT;
        if (ans <= alpha)
            e = Transposition::UPPER;
        else if (ans >= beta)
            e = Transposition::LOWER;
        trans.put(my_hash, ans, e, best_move, depth);
    }

    if (ans >= beta && best_move != 0 && !board.is_capture(best_move))
        update_history(depth, best_move, searched_moves, searched_moves_count);

    if (result)
        *result = best_move;

    return ans;
}

int Agent::quiescence(Board &board, int side, int alpha, int beta, POSITION last_square)
{
    bool store_tt;
    return quiescence(board, side, alpha, beta, board.in_check(side), last_square, &store_tt);
}

int Agent::quiescence(Board &board, int side, int alpha, int beta,
        bool in_check, POSITION last_square, bool *store_tt)
{
    *store_tt = true;

    int ans, sv = board.static_value(side);
    if (in_check)
        ans = -INF;
    else
        ans = sv;

    if (ans < beta)
    {
        MOVE moves[120];
        int capture_scores[120], moves_count = 0;
        board.generate_moves(side, moves, capture_scores, &moves_count);

        if (!in_check)
        {
            int c = 0;
            for (int i = 0; i < moves_count; ++i)
            {
                if (move_dst(moves[i]) == last_square)
                    capture_scores[i] = max(capture_scores[i], Board::KING_CAPTURE_VALUE - 1);
                if (capture_scores[i] > Board::NON_CAPTURE
                        && is_winning_capture(&board, moves[i], capture_scores[i], side))
                {
                    MOVE tm = moves[c];
                    moves[c] = moves[i];
                    moves[i] = tm;
                    int t = capture_scores[c];
                    capture_scores[c] = capture_scores[i];
                    capture_scores[i] = t;

                    ++c;
                }
            }
            order_moves(moves, capture_scores, c, c);
            if (!CHECKS_IN_QUIESCENCE)
                moves_count = c;
        }

        for (int i = 0; ans < beta && i < moves_count; ++i)
        {
            MoveType mt;
            if (!board.move(moves[i], &mt))
                continue;

            int t;
            bool propagated_store;
            if (!special_move_type(mt, &t, &propagated_store))
            {
                t = -INF;
                // it is faster to refute suicide here
                if (in_check && board.in_check(side))
                    propagated_store = true;
                else
                {
                    bool next_in_check = board.in_check(1 - side);

                    if (in_check || capture_scores[i] > Board::NON_CAPTURE || next_in_check)
                    {
                        int current_alpha = max(alpha, ans);
                        t = -quiescence(board, 1 - side, -beta, -current_alpha,
                                next_in_check, move_dst(moves[i]), &propagated_store);
                    }
                }
            }

            if (t > ans)
            {
                *store_tt = propagated_store;
                ans = t;
            }

            board.unmove();
        }
    }

    return ans;
}

void Agent::update_history(int depth, MOVE best_move, MOVE *searched_moves, int count)
{
    int delta = depth * depth;
    for (int i = 0; i < count; ++i)
    {
        if (searched_moves[i] != best_move)
            move_score[searched_moves[i]] -= delta;
        else
            move_score[searched_moves[i]] += delta;
    }
}
