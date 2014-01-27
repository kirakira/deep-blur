#include <iostream>

#include "agent.h"

using namespace std;

int Agent::search(Board &board, int side, MOVE *result)
{
    firstHit = 0;
    secondHit = 0;
    miss = 0;

    int ret = alpha_beta(board, side, result, 5, -INF, INF, true);

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
        MOVE best_move;
        // Null-move
        if (nullable)
        {
            ans = -alpha_beta(board, 1 - side, &best_move, depth - 1, -beta, -alpha, false);
        }

        if (ans < beta)
        {
            MOVE moves[120];
            int moves_count;
            board.generate_moves(side, moves, &moves_count);

            int bestIndex = 0;
            for (int i = 0; i < moves_count; ++i)
            {
                MOVE move = moves[i];
                if (!board.move(move))
                    continue;

                int current_alpha = max(alpha, ans);
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
