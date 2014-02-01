#include <iostream>
#include <algorithm>
#include <cstring>
#include <ctime>

#include "agent.h"

using namespace std;

Agent::Agent()
    : trans(22)
{
}

void Agent::order_moves(MOVE *moves, int *scores, int moves_count, int order_count)
{
    while (order_count > 0 && moves_count > 0)
    {
        int best = scores[0], besti = 0;
        for (int i = 1; i < moves_count; ++i)
            if (scores[i] > Board::NON_CAPTURE && scores[i] > best)
            {
                best = scores[i];
                besti = i;
            }
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

    for (int level = 0; level <= depth; ++level)
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

// if return value >= beta, it is a lower bound; if return value <= alpha, it is an upper bound
int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable)
{
    int his_score, his_exact = 0, his_depth;
    MOVE his_move = 0;
    bool t_hit = false;
    if (trans.get(board.hash_code(side), &his_score, &his_exact, &his_move, &his_depth)
            && (his_move == 0 || board.checked_move(his_move)))
    {
        if (his_move != 0)
            board.unmove();

        t_hit = true;

        if (his_depth >= depth &&
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

    if (depth >= 2 && !(t_hit && his_depth >= depth / 2 && his_move != 0))
        alpha_beta(board, side, &his_move, depth / 2, alpha, beta, false);

    if (depth == 0)
    {
        ++leaf;
        ans = board.static_value(side);
    }
    else
    {
        if (nullable)
            ans = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -beta + 1, false);
        bool game_end;

        if (ans < beta)
        {
            ans = -INF;

            if (his_move != 0 && board.move(his_move, &game_end))
            {
                if (game_end)
                    ans = INF;
                else
                    ans = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -alpha, true);
                board.unmove();

                best_move = his_move;
                first_ans = ans;
            }

            if (ans < beta)
            {
                MOVE moves[120];
                int capture_scores[120], moves_count;

                board.generate_moves(side, moves, capture_scores, &moves_count);
                order_moves(moves, capture_scores, moves_count, 1);

                int history_scores[120];
                for (int i = 0; i < moves_count; ++i)
                    history_scores[i] = move_score[moves[i]];

                order_moves(moves + 1, history_scores + 1, moves_count - 1, 2);

                for (int i = 0; i < moves_count; ++i)
                {
                    if (moves[i] == his_move)
                        continue;

                    MOVE move = moves[i];
                    if (!board.move(move, &game_end))
                        continue;

                    int t;
                    if (game_end)
                        t = INF;
                    else
                    {
                        int current_alpha = max(alpha, ans);
                        t = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -current_alpha, true);
                    }
                    board.unmove();

                    if (his_move == 0 && i == 0)
                        first_ans = t;

                    if (t > ans)
                    {
                        best_move = move;
                        ans = t;
                    }

                    if (t >= beta)
                    {
                        if (depth > 0)
                            move_score[move] += depth * depth;
                        if (his_move == 0 && i == 0)
                            ++first_cut;
                        break;
                    }
                }
            }
            else
                ++first_cut;
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
