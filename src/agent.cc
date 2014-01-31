#include <iostream>
#include <algorithm>
#include <cstring>

#include "agent.h"

using namespace std;

bool Agent::MoveComparator::operator()(const MOVE &x, const MOVE &y) const
{
    if (move_dst(x) == king_pos)
        return true;
    if (move_dst(y) == king_pos)
        return false;
    if (y == transp_move)
        return false;
    if (x == transp_move)
        return true;
    return score_table[x] > score_table[y];
}

void Agent::MoveComparator::set(int *table)
{
    score_table = table;
}

void Agent::MoveComparator::set(MOVE trans_move, POSITION king_p)
{
    transp_move = trans_move;
    king_pos = king_p;
}

Agent::Agent()
    : trans(22)
{
}

int Agent::search(Board &board, int side, MOVE *result, int depth)
{
    firstHit = 0;
    secondHit = 0;
    miss = 0;

    int ret = id(board, side, result, depth);

    int tot = firstHit + secondHit + miss;
    cout << "# move ordering: " << (double) firstHit / (double) tot << " " << (double) secondHit / (double) tot << " " << (double) miss / (double) tot << endl;
    cout << "# nodes: " << nodes << ", transp hit: " << trans_hit << " (" << (double) trans_hit / (double) (trans_hit + nodes) << ")" << endl;
    trans.stat();
    return ret;
}

int Agent::id(Board &board, int side, MOVE *result, int depth)
{
    int ret;
    memset(move_score, 0, sizeof(move_score));
    move_comparator.set(move_score);
    trans_hit = 0;
    nodes = 0;
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
                cout << score << ") \t";

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

// if return value >= beta, it is a lower bound; if return value <= alpha, it is an upper bound
int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable)
{
    int his_score, his_exact = 0, his_depth;
    MOVE his_move = 0;
    if (trans.get(board.hash_code(side), &his_score, &his_exact, &his_move, &his_depth)
            && his_depth >= depth &&
            (his_exact == Transposition::EXACT || (his_exact == Transposition::UPPER && his_score <= alpha)
             || (his_exact == Transposition::LOWER && his_score >= beta))
            && ((nullable && his_move == 0) || (his_move != 0 && board.checked_move(his_move))))
    {
        if (his_move != 0)
            board.unmove();

        ++trans_hit;

        if (result)
            *result = his_move;
        return his_score;
    }
    ++nodes;

    int ans = -INF;
    MOVE best_move = 0;

    if (depth == 0)
        ans = board.static_value(side);
    else
    {
        if (nullable)
            ans = -alpha_beta(board, 1 - side, NULL, depth - 1, -beta, -beta + 1, false);
        if (ans < beta)
        {
            ans = -INF;

            MOVE moves[120];
            int moves_count;

            board.generate_moves(side, moves, &moves_count);
            move_comparator.set(his_move, board.king_position(1 - side));
            sort(moves, moves + moves_count, move_comparator);

            int bestIndex = 0;
            for (int i = 0; i < moves_count; ++i)
            {
                MOVE move = moves[i];
                bool game_end;
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

                if (t > ans)
                {
                    bestIndex = i;
                    best_move = move;
                    ans = t;
                }

                if (t >= beta)
                {
                    if (depth > 0)
                        move_score[move] += depth * depth;
                    break;
                }
            }
            if (bestIndex == 0)
                ++firstHit;
            else if (bestIndex == 1)
                ++secondHit;
            else
                ++miss;
        }
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

    if (result)
        *result = best_move;
    return ans;
}
