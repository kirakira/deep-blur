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

    int ret = alpha_beta(board, side, result, 6, -INF, INF, true);

    int tot = firstHit + secondHit + miss;
    cout << (double) firstHit / (double) tot << " " << (double) secondHit / (double) tot << " " << (double) miss / (double) tot << endl;
    return ret;
}

int Agent::alpha_beta(Board &board, int side, MOVE *result, int depth, int alpha, int beta, bool nullable)
{
    int ans = -INF;

    if (depth == 0)
        ans = board.static_value(side);
    else
    {
        if (ans < beta)
        {
            MOVE moves[120];
            int moves_count;
            board.generate_moves(side, moves, &moves_count);
            sort(moves, moves + moves_count, move_comparator);

            int bestIndex = 0;
            for (int i = 0; i < moves_count; ++i)
            {
                MOVE move = moves[i];
                if (!board.move(move))
                    continue;

                int current_alpha = max(alpha, ans);
                MOVE best_move;
                int t = -alpha_beta(board, 1 - side, &best_move, depth - 1, -beta, -current_alpha, true);
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
