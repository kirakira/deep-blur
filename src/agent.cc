#include <iostream>
#include <cstring>
#include <ctime>

#include "agent.h"

using namespace std;

Agent::Agent()
    : trans(22)
{
}

int Agent::select_best_move(MOVE *moves, int *scores, int moves_count)
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
        int besti = select_best_move(moves, scores, moves_count);
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

int Agent::search(Board &board, int side, MOVE *result, int depth)
{
    trans_hit = 0;
    nodes = 0;
    leaf = 0;
    null_cut = 0;
    first_cut = 0;
    beta_cut = 0;
    alpha_nodes = 0;
    first_best = 0;

    clock_t t = clock();
    int ret = id(board, side, result, depth);
    t = clock() - t;

    double sec = (double) t / (double) CLOCKS_PER_SEC;
    cout << "# transposition hit rate: " << (double) trans_hit * 100 / (double) (trans_hit + nodes) << "%" << endl;
    cout << "# null-move cutoff: " << (double) null_cut * 100 / (double) (nodes - leaf)
        << "%, first-move cutoff: " << (double) first_cut * 100 / (double) (nodes - leaf)
        << "%, beta cutoff: " << (double) beta_cut * 100 / (double) (nodes - leaf)
        << "%, alpha nodes: " << (double) alpha_nodes * 100/ (double) (nodes - leaf) << "%" << endl;
    cout << "# first-move-best rate: " << (double) first_best * 100 / (double) (nodes - null_cut - leaf) << "%" << endl;
    trans.stat();
    cout << "# total nodes: " << nodes << ", NPS: " << (double) nodes / sec / 1000000. << "m in " << sec << "s" << endl;

    return ret;
}

int Agent::id(Board &board, int side, MOVE *result, int depth)
{
    int ret;
    memset(move_score, 0, sizeof(move_score));

    for (int level = 1; level <= depth; ++level)
    {
        ret = alpha_beta(board, side, result, level, -INF, INF, false);

        cout << "# Level " << level << ": ";
        int score, exact, d, s = side, count = 0;
        MOVE t;
        while (trans.get(board.hash_code(s), &score, &exact, &t, &d) && count < level)
        {
            if (d > 0 && d >= level - count && (t == 0 || board.checked_move(t)))
            {
                if (t == 0)
                    cout << "null";
                else
                {
                    ++count;
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
        while (count > 0)
        {
            --count;
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
int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable)
{
    int his_score, his_exact = 0, his_depth;
    MOVE his_move = 0;
    bool t_hit = false, rep;
    if (trans.get(board.hash_code(side), &his_score, &his_exact, &his_move, &his_depth)
            && (his_move == 0 || board.checked_move(his_move, &rep)))
    {
        if (his_move != 0)
            board.unmove();

        t_hit = true;

        if (his_depth >= depth &&
            (his_exact == Transposition::EXACT || (his_exact == Transposition::UPPER && his_score <= alpha)
             || (his_exact == Transposition::LOWER && his_score >= beta))
            && (nullable || (his_move != 0 && !rep)))
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

    if (depth >= 3 && !(t_hit && his_depth >= depth - 2 && his_move != 0 && !rep))
        alpha_beta(board, side, &his_move, depth - 2, alpha, beta, false);

    if (depth == 0)
    {
        ++leaf;
        ans = quiescence(board, side, alpha, beta);
    }
    else
    {
        if (nullable)
            ans = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -beta + 1, false);

        if (ans < beta)
        {
            ans = -INF;

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

                    order_moves(moves + start, capture_scores + start, moves_count, 1);

                    for (int i = 0; i < moves_count; ++i)
                        history_scores[i + start] = move_score[moves[i + start]];
                    if (capture_scores[start] < 35)
                        order_moves(moves + start, history_scores + start, moves_count, 5);

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
                    if (i <= 2)
                        t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha, true);
                    else
                    {
                        t = -alpha_beta(board, 1 - side, NULL, depth - 1, -current_alpha - 1, -current_alpha, true);
                        if (current_alpha < t && t < beta)
                            t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha, true);
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
                    if (depth > 0 && move != his_move)
                        move_score[move] += depth * depth;
                    if (i == 0)
                        ++first_cut;
                    break;
                }
            }
        }
        else
            ++null_cut;
    }

    if (his_exact != Transposition::EXACT || his_depth <= depth)
    {
        int e = Transposition::EXACT;
        if (ans <= alpha)
            e = Transposition::UPPER;
        else if (ans >= beta)
            e = Transposition::LOWER;
        trans.put(board.hash_code(side), ans, e, best_move, depth);
    }

    if (depth > 0)
    {
        if (ans >= beta)
            ++beta_cut;
        else if (ans <= alpha)
            ++alpha_nodes;
        if (first_ans == ans)
            ++first_best;
    }

    if (result)
        *result = best_move;
    return ans;
}

int Agent::quiescence(Board &board, int side, int alpha, int beta)
{
    vector<uint64_t> rep[2];
    int last_progress[2] = {-1, -1};
    return quiescence(board, side, alpha, beta, rep, last_progress, board.in_check(side), INVALID_POSITION);
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

    return v < ret;
}

int Agent::static_exchange_eval(Board &board, int side, POSITION pos)
{
    MOVE moves[120];
    int capture_scores[120], moves_count;
    board.generate_moves(side, moves, capture_scores, &moves_count);

    int besti = -1;
    for (int i = 0; i < moves_count; ++i)
    {
        if (move_dst(moves[i]) == pos && capture_scores[i] > Board::NON_CAPTURE)
        {
            if (besti == -1 || capture_scores[i] > capture_scores[besti])
                besti = i;
        }
    }

    int ans = board.static_value(side);
    if (besti == -1 || !board.move(moves[besti]))
        return ans;

    int ret = -static_exchange_eval(board, 1 - side, pos);

    board.unmove();

    return max(ans, ret);
}

int Agent::quiescence(Board &board, int side, int alpha, int beta, vector<uint64_t> *rep, int *last_progress, bool in_check, POSITION last_pos)
{
    for (int i = last_progress[side] + 1; i < rep[side].size(); ++i)
        if (rep[side][i] == board.hash_code(side))
            return 0;

    int ans;
    if (in_check)
        ans = -INF;
    else
        ans = board.static_value(side);

    if (ans >= beta)
        return ans;

    MOVE moves[120];
    int capture_scores[120], moves_count = 0;
    board.generate_moves(side, moves, capture_scores, &moves_count);

    if (!in_check)
    {
        int c = 0;
        for (int i = 0; i < moves_count; ++i)
            if (capture_scores[i] > Board::NON_CAPTURE)
            {
                MOVE tm = moves[c];
                moves[c] = moves[i];
                moves[i] = tm;
                int t = capture_scores[c];
                capture_scores[c] = capture_scores[i];
                capture_scores[i] = t;

                ++c;
            }
        moves_count = c;
        order_moves(moves, capture_scores, moves_count, c);
    }

    for (int i = 0; i < moves_count; ++i)
    {
        bool next_in_check;
        if (!(in_check ||
                (capture_scores[i] > Board::NON_CAPTURE
                && ((move_dst(moves[i]) == last_pos)
                || is_winning_capture(board, moves[i], capture_scores[i], side)))))
            continue;
        if (!board.move(moves[i]))
            continue;
        if (board.in_check(side))
        {
            board.unmove();
            continue;
        }

        next_in_check = board.in_check(1 - side);

        int saved_progress = last_progress[1 - side];
        if (capture_scores[i] > Board::NON_CAPTURE)
            last_progress[1 - side] = rep[1 - side].size();
        rep[side].push_back(board.hash_code(side));

        int current_alpha = max(alpha, ans);
        int t = -quiescence(board, side, -beta, -current_alpha, rep, last_progress, next_in_check, move_dst(moves[i]));

        rep[side].pop_back();
        last_progress[1 - side] = saved_progress;
        board.unmove();

        if (t >= ans)
            ans = t;

        if (t >= beta)
            break;
    }

    return ans;
}
