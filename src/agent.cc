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

int Agent::search(Board &board, int side, MOVE *result, int time_limit, int depth)
{
    trans_hit = 0;
    nodes = 0;
    leaf = 0;
    null_cut = 0;
    first_cut = 0;
    beta_nodes = 0;
    alpha_nodes = 0;
    first_best = 0;

    search_start_time = clock();
    clock_t deadline = search_start_time + (double) time_limit / 1000. * (double) CLOCKS_PER_SEC;
    int ret = id(board, side, result, deadline, &depth);
    clock_t t = clock() - search_start_time;

    double sec = (double) t / (double) CLOCKS_PER_SEC;
    cout << "# transposition hit rate: " << (double) trans_hit * 100 / (double) (trans_hit + nodes) << "%" << endl;
    cout << "# null-move cutoff: " << (double) null_cut * 100 / (double) (nodes - leaf)
        << "%, first-move cutoff: " << (double) first_cut * 100 / (double) (nodes - leaf) << "%" << endl;
    cout << "# beta nodes: " << (double) beta_nodes * 100 / (double) (nodes)
        << "%, alpha nodes: " << (double) alpha_nodes * 100/ (double) (nodes)
        << "%, leafs: " << (double) leaf * 100 / (double) (nodes) << "%" << endl;
    cout << "# first-move-best rate: " << (double) first_best * 100 / (double) (nodes - null_cut - leaf) << "%" << endl;
    trans.stat();
    cout << "# total nodes: " << (double) nodes / 1e6
        << "m, EBF: " << ebf(nodes, depth)
        << ", NPS: " << (double) nodes / sec / 1000000. << "m in " << sec << "s" << endl;

    return ret;
}

void Agent::output_thinking(int ply, int score, PV *pv)
{
    int t = (int) ((double) (clock() - search_start_time) / (double) CLOCKS_PER_SEC * 100);
    cout << ply << "\t" << score << "\t" << t << "\t" << readable_number(nodes) << "\t";
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

    for (int level = 1; level <= *depth; ++level)
    {
        PV pv;
        pv.count = 0;

        bool aborted;
        MOVE current_move;

        int t = search_root(board, side, &current_move, level, deadline, &pv, &aborted);
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

int Agent::search_root(Board &board, int side, MOVE *result, int depth, clock_t deadline, PV *pv, bool *aborted)
{
    int his_score, his_exact = 0, his_depth = 0;
    MOVE his_move = 0;
    bool rep;
    if (USE_TRANS_TABLE && trans.get(board.hash_code(side), &his_score, &his_exact, &his_move, &his_depth)
            && (his_move == 0 || board.checked_move(side, his_move, &rep)))
    {
        if (his_move != 0)
            board.unmove();
    }

    if (USE_IID && depth >= 6)
        alpha_beta(board, side, &his_move, depth - 2, -INF, INF, 1, deadline, false, INVALID_POSITION, true, NULL);

    MoveList ml(&board, side, his_move, move_score, 0, 0);

    *aborted = false;
    MOVE move, best_move = 0;
    int ans = ABORTED;

    for (int i = 0; ans < INF && (move = ml.next_move()); ++i)
    {
        bool game_end, rep_attack;
        if (!board.move(move, &game_end, &rep_attack) || rep_attack)
        {
            if (rep_attack)
                board.unmove();
            continue;
        }

        PV newPV;
        newPV.moves[0] = move;
        newPV.count = 1;
        int t;
        if (game_end)
            t = INF;
        else
        {
            POSITION dst = move_dst(move);

            if (i == 0)
                t = -alpha_beta(board, 1 - side, NULL, depth - 1, -INF, -ans,
                        1, deadline, true, dst, true, &newPV);
            else
            {
                t = -alpha_beta(board, 1 - side, NULL, depth - 1, -ans - 1,
                        -ans, 1, deadline, true, dst, false, NULL);
                if (ans < t)
                    t = -alpha_beta(board, 1 - side, NULL, depth - 1, -INF, -ans,
                            1, deadline, true, dst, true, &newPV);
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
            if (pv)
            {
                pv->count = 0;
                catPV(pv, &newPV);
            }

            output_thinking(depth, ans, pv);
        }
    }

    if (USE_TRANS_TABLE)
    {
        int e;
        if (*aborted)
            e = Transposition::LOWER;
        else
            e = Transposition::EXACT;
        trans.put(board.hash_code(side), ans, e, best_move, depth);
    }

    if (best_move != 0 && !board.is_capture(best_move))
        move_score[best_move] += depth * depth;

    if (result)
        *result = best_move;
    return ans;
}

// We don't detect repetitions other than repeated attacks in Board::move
// since an ordinary repetition detection is not compatible with transposition tables

// if return value >= beta, it is a lower bound; if return value <= alpha, it is an upper bound
int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta,
        int ply, clock_t deadline, bool nullable, POSITION last_square, bool isPV, PV *pv)
{
    int his_score, his_exact = 0, his_depth = 0;
    MOVE his_move = 0;
    bool rep;
    if (USE_TRANS_TABLE && trans.get(board.hash_code(side), &his_score, &his_exact, &his_move, &his_depth)
            && (his_move == 0 || board.checked_move(side, his_move, &rep)))
    {
        if (his_move != 0)
            board.unmove();

        if (his_depth >= depth && (his_move == 0 || !rep) &&
                (his_exact == Transposition::EXACT || (his_exact == Transposition::UPPER && his_score <= alpha)
                 || (his_exact == Transposition::LOWER && his_score >= beta))
                && (nullable || his_move != 0))
        {
            ++trans_hit;

            if (result)
                *result = his_move;
            return his_score;
        }

        if (his_exact == Transposition::UPPER && his_score <= beta - 1)
            nullable = false;
    }

    ++nodes;

    if (nodes % CHECK_TIME_NODES == 0 && clock() >= deadline)
        return ABORTED;

    int ans = -INF, first_ans = ABORTED;
    MOVE best_move = 0;

    if (depth == 0)
    {
        ++leaf;
        ans = quiescence(board, side, alpha, beta, last_square);
    }
    else
    {
        if (USE_NULL_MOVE && nullable && !isPV)
            ans = -alpha_beta(board, 1 - side, NULL, max(0, depth - 3),
                    -beta, -beta + 1, ply, deadline, false, INVALID_POSITION, isPV, NULL);

        if (ans == -ABORTED)
            return ABORTED;

        if (ans < beta)
        {
            ans = -INF;

            if (USE_IID && depth >= 6)
                alpha_beta(board, side, &his_move, depth - 2, alpha, beta, ply + 1, deadline, false, last_square, isPV, NULL);

            int original_pv_count = pv ? pv->count : 0;
            MoveList ml(&board, side, his_move, move_score, killer[ply][0], killer[ply][1]);
            MOVE move;
            for (int i = 0; ans < beta && (move = ml.next_move()); ++i)
            {
                bool game_end, rep_attack;
                if (!board.move(move, &game_end, &rep_attack) || rep_attack)
                {
                    if (rep_attack)
                        board.unmove();
                    continue;
                }

                PV newPV;
                newPV.moves[0] = move;
                newPV.count = 1;
                int t;
                if (game_end)
                    t = INF;
                else
                {
                    int current_alpha = max(alpha, ans);
                    POSITION dst = move_dst(move);

                    if (i == 0)
                        t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha,
                                ply + 1, deadline, true, dst, isPV, &newPV);
                    else
                    {
                        t = current_alpha + 1;

                        if (USE_LMR && ply > LMR_PLY && i > LMR_NODES && !board.is_capture(move))
                            t = -alpha_beta(board, 1 - side, NULL, depth - 2, -current_alpha - 1,
                                    -current_alpha, ply + 1, deadline, true, dst, false, NULL);

                        if (t > current_alpha)
                            t = -alpha_beta(board, 1 - side, NULL, depth - 1, -current_alpha - 1,
                                    -current_alpha, ply + 1, deadline, true, dst, false, NULL);
                        if (current_alpha < t && t < beta)
                        {
#ifdef DEBUG_OUTPUT
                            int t0 = t;
#endif
                            t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha,
                                    ply + 1, deadline, true, dst, isPV, &newPV);

#ifdef DEBUG_OUTPUT
                            board.print();
                            cout << "(alpha, beta) =(" << current_alpha << ", " << beta << ")" << endl;
                            cout << "move index: " << i << ", " << move_string(moves[i]) << endl;
                            cout << "first returned: " << t0 << endl;
                            cout << "second returned: " << t << endl;
                            for (int j = 0; j < i; ++j)
                                cout << move_string(moves[j]) << "(" << move_score[moves[j]] << ") ";
                            cout << endl << endl;
#endif
                        }
                    }
                }
                board.unmove();

                if (t == -ABORTED)
                    return ABORTED;

                if (i == 0)
                    first_ans = t;

                if (t > ans)
                {
                    best_move = move;
                    ans = t;
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
                    if (i == 0)
                        ++first_cut;
                    break;
                }
            }
        }
        else
            ++null_cut;
    }

    if (USE_TRANS_TABLE && (his_exact != Transposition::EXACT || his_depth <= depth))
    {
        int e = Transposition::EXACT;
        if (ans <= alpha)
            e = Transposition::UPPER;
        else if (ans >= beta)
            e = Transposition::LOWER;
        trans.put(board.hash_code(side), ans, e, best_move, depth);
    }

    if (ans >= beta)
        ++beta_nodes;
    else if (ans <= alpha)
        ++alpha_nodes;
    if (first_ans == ans)
        ++first_best;

    if (best_move != 0 && !board.is_capture(best_move))
        move_score[best_move] += depth * depth;

    if (result)
        *result = best_move;
    return ans;
}

int Agent::quiescence(Board &board, int side, int alpha, int beta, POSITION last_square)
{
    vector<uint64_t> rep[2];
    int last_progress[2] = {-1, -1};
    return quiescence(board, side, alpha, beta, rep, last_progress, board.in_check(side), last_square);
}

int Agent::quiescence(Board &board, int side, int alpha, int beta, vector<uint64_t> *rep,
        int *last_progress, bool in_check, POSITION last_square)
{
    uint64_t my_hash = board.hash_code(side);
    for (size_t i = last_progress[side] + 1; i < rep[side].size(); ++i)
        if (rep[side][i] == board.hash_code(side))
            return 0;

    rep[side].push_back(my_hash);

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
                    capture_scores[i] = max(capture_scores[i], Board::KING_CAPTURE - 1);
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
            bool game_end, rep_attack;
            if (!board.move(moves[i], &game_end, &rep_attack))
                continue;

            // it is faster to refute suicide here
            if (rep_attack || game_end || (in_check && board.in_check(side)))
            {
                if (game_end)
                    ans = INF;
                board.unmove();
                continue;
            }

            bool next_in_check = board.in_check(1 - side);

            if (in_check || capture_scores[i] > Board::NON_CAPTURE || next_in_check)
            {
                int saved_progress = last_progress[1 - side];
                if (capture_scores[i] > Board::NON_CAPTURE)
                    last_progress[1 - side] = rep[1 - side].size();

                int current_alpha = max(alpha, ans);
                int t = -quiescence(board, 1 - side, -beta, -current_alpha, rep,
                        last_progress, next_in_check, move_dst(moves[i]));

                last_progress[1 - side] = saved_progress;

                if (t >= ans)
                    ans = t;
            }

            board.unmove();
        }
    }
    rep[side].pop_back();

    return ans;
}
