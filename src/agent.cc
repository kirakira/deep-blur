#include "agent.h"

using namespace std;

int Agent::search(Board &board, int side, Move *result)
{
    return alpha_beta(board, side, result, 5, -INF, INF);
}

int Agent::alpha_beta(Board &board, int side, Move *result, int depth, int alpha, int beta)
{
    int ans = -INF;
    if (depth == 0)
        ans = board.static_value(side);
    else
    {
        Move moves[120];
        int moves_count;
        board.generate_moves(side, moves, &moves_count);

        for (int i = 0; i < moves_count; ++i)
        {
            Move move = moves[i];
            if (!board.move(move))
                continue;

            int current_alpha = max(alpha, ans);
            Move best_move;
            int t = -alpha_beta(board, 1 - side, &best_move, depth - 1, -beta, -current_alpha);
            board.unmove();

            if (t > ans)
            {
                ans = t;
                *result = move;
            }

            if (t >= beta)
            {
                break;
            }
        }
    }

    return ans;
}
