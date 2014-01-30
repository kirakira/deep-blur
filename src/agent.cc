#include <iostream>
#include <algorithm>
#include <cstring>

#include "agent.h"

using namespace std;

Agent::MoveComparator::MoveComparator(int *table)
    : score_table(table)
{
}

bool Agent::MoveComparator::operator()(const MOVE &x, const MOVE &y) const
{
    return score_table[x] > score_table[y];
}

Agent::Agent()
    : move_comparator(move_score)
{
}

int Agent::search(Board &board, int side, MOVE *result)
{
    firstHit = 0;
    secondHit = 0;
    miss = 0;
    memset(move_score, 0, sizeof(move_score));

    int ret = alpha_beta(board, side, result, 6, -INF, INF, false);

    //int tot = firstHit + secondHit + miss;
    //cout << (double) firstHit / (double) tot << " " << (double) secondHit / (double) tot << " " << (double) miss / (double) tot << endl;
    return ret;
}

// Null-move heuristic: 40%

// if return value >= beta, it is a lower bound; if return value <= alpha, it is an upper bound
int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable)
{
    int ans = -INF;

    if (depth == 0)
        ans = board.static_value(side);
    else
    {
        // TODO don't perform null-move when in check
        MOVE best_move;
        if (nullable)
            ans = -alpha_beta(board, 1 - side, &best_move, depth - 1, -beta, -beta + 1, false);
        if (ans < beta)
        {
            ans = -INF;

            MOVE moves[120];
            int moves_count;
            board.generate_moves(side, moves, &moves_count);
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
                    t = -alpha_beta(board, 1 - side, &best_move, depth - 1, -beta, -current_alpha, true);
                }
                board.unmove();

                if (t > ans)
                {
                    bestIndex = i;
                    ans = t;
                    *result = move;
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

    return ans;
}
