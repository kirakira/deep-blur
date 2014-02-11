#include <iostream>
#include <cstring>
#include <ctime>
#include <cmath>

#include "agent.h"

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

int Agent::search(Board &board, int side, MOVE *result, int depth)
{
    trans_hit = 0;
    nodes = 0;
    leaf = 0;
    null_cut = 0;
    first_cut = 0;
    beta_nodes = 0;
    alpha_nodes = 0;
    first_best = 0;

    clock_t t = clock();
    int ret = id(board, side, result, depth);
    t = clock() - t;

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

int Agent::id(Board &board, int side, MOVE *result, int depth)
{
    int ret = 0;
    memset(move_score, 0, sizeof(move_score));
    memset(killer, 0, sizeof(killer));

    for (int level = 1; level <= depth; ++level)
    {
        ret = alpha_beta(board, side, result, level, -INF, INF, 0, false, INVALID_POSITION);

        cout << "# Level " << level << ": ";
        int score, exact, d, s = side, non_null_count = 0, count = 0;
        MOVE t;
        while (trans.get(board.hash_code(s), &score, &exact, &t, &d) && count < level)
        {
            if (d > 0 && d >= level - count && (t == 0 || board.checked_move(s, t)))
            {
                ++count;
                if (t == 0)
                    cout << "null";
                else
                {
                    ++non_null_count;
                    cout << move_string(t);
                }
                cout << "(" << d << ": ";
                if (exact == Transposition::LOWER)
                    cout << ">=";
                else if (exact == Transposition::UPPER)
                    cout << "<=";
                cout << score << ")\t";

                s = 1 - s;
            }
            else
                break;
        }
        cout << endl;
        while (non_null_count > 0)
        {
            --non_null_count;
            board.unmove();
        }
    }
    return ret;
}

// Null-move heuristic: 40%
// Transposition: 30%
// Iterative deepening: less than 3%
// Transposition move as move ordering: 5%
// Internal iterative deepening: 10%
// Not tring null-move if transposition table suggests it's unlikely to fail-high: 10%
// Trying history move first, then if not cut-off, generate remaining moves
//
// We don't detect repetitions other than repeated attacks in Board::move
// since an ordinary repetition detection is not compatible with transposition tables

// if return value >= beta, it is a lower bound; if return value <= alpha, it is an upper bound
int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta,
        int ply, bool nullable, POSITION last_square)
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

    int ans = -INF, first_ans = ans;
    MOVE best_move = 0;
    bool isPV = (beta > alpha + 1);

    if (depth == 0)
    {
        ++leaf;
        ans = quiescence(board, side, alpha, beta, last_square);
    }
    else
    {
        if (USE_NULL_MOVE && nullable && !isPV)
            ans = -alpha_beta(board, 1 - side, NULL, max(0, depth - 3),
                    -beta, -beta + 1, ply, false, INVALID_POSITION);

        if (ans < beta)
        {
            ans = -INF;

            if (USE_IID && depth >= 6)
                alpha_beta(board, side, &his_move, depth - 2, alpha, beta, ply + 1, false, last_square);

            bool moves_generated = false;
            MOVE moves[120];
            int moves_count = 0, capture_scores[120], history_scores[120];

            if (his_move != 0)
                moves[moves_count++] = his_move;

            for (int i = 0; ans < beta && (!moves_generated || i < moves_count); ++i)
            {
                if (i == moves_count)
                {
                    int start = moves_count;
                    board.generate_moves(side, moves + start,
                            capture_scores + start, &moves_count);

                    for (int j = start; j < moves_count; ++j)
                        if (!(capture_scores[j] > Board::NON_CAPTURE
                                && is_winning_capture(board, moves[j], capture_scores[j], side)))
                            capture_scores[j] = Board::NON_CAPTURE;
                    order_moves(moves + start, capture_scores + start, moves_count, moves_count);

                    int c = start;
                    while (c < moves_count && capture_scores[c] > Board::NON_CAPTURE)
                        ++c;

                    for (int j = c; j < moves_count; ++j)
                    {
                        if (USE_KILLER)
                        {
                            if (moves[j] == killer[ply][0])
                                history_scores[j] = 1000000;
                            else if (moves[j] == killer[ply][1])
                                history_scores[j] = 999999;
                            else
                                history_scores[j] = move_score[moves[j]];
                        }
                        else
                            history_scores[j] = move_score[moves[j]];
                    }
                    order_moves(moves + c, history_scores + c, moves_count - c, moves_count - c);

                    moves_count += start;
                    moves_generated = true;
                }

                if (moves_generated && moves[i] == his_move)
                    continue;

                MOVE move = moves[i];
                bool game_end, rep_attack;
                if (!board.move(move, &game_end, &rep_attack) || rep_attack)
                {
                    if (rep_attack)
                        board.unmove();
                    continue;
                }

                int t;
                if (game_end)
                    t = INF;
                else
                {
                    int current_alpha = max(alpha, ans);
                    POSITION dst = move_dst(move);

                    if (i <= 0)
                        t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha, ply + 1, true, dst);
                    else
                    {
                        t = current_alpha + 1;

                        if (USE_LMR && ply > LMR_PLY && i > LMR_NODES &&
                                capture_scores[i] <= Board::NON_CAPTURE)
                            t = -alpha_beta(board, 1 - side, NULL, depth - 2, -current_alpha - 1,
                                    -current_alpha, ply + 1, true, dst);

                        if (t > current_alpha)
                            t = -alpha_beta(board, 1 - side, NULL, depth - 1, -current_alpha - 1,
                                    -current_alpha, ply + 1, true, dst);
                        if (current_alpha < t && t < beta)
                        {
#ifdef DEBUG_OUTPUT
                            int t0 = t;
#endif
                            t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha,
                                    ply + 1, true, dst);

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

                if (i == 0)
                    first_ans = t;

                if (t > ans)
                {
                    best_move = move;
                    ans = t;
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

    if (best_move != 0)
        move_score[best_move] += 1;

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

bool Agent::is_winning_capture(Board &board, MOVE move, int score, int side)
{
    int captured = (score >> 3), capturing = 8 - (score & 7);
    if (captured > capturing)
        return true;

    int v = board.static_value(side);

    bool game_end;
    if (!board.move(move, &game_end))
        return false;
    if (game_end)
    {
        board.unmove();
        return true;
    }

    int ret = -static_exchange_eval(board, 1 - side, move_dst(move));

    board.unmove();

    return v <= ret;
}

int Agent::static_exchange_eval(Board &board, int side, POSITION pos)
{
    int ans = board.static_value(side);

    MOVE response;
    bool game_end;
    if (!board.is_attacked(pos, true, &response) || !board.move(response, &game_end))
        return ans;

    int ret;
    if (game_end)
        ret = INF;
    else
        ret = -static_exchange_eval(board, 1 - side, pos);

    board.unmove();

    return max(ans, ret);
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
                        && is_winning_capture(board, moves[i], capture_scores[i], side))
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
