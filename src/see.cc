#include <algorithm>

#include "see.h"
#include "common.h"

using namespace std;

bool is_winning_capture(Board *board, MOVE move, int score, int side)
{
    int captured = (score >> 3), capturing = 8 - (score & 7);
    if (captured > capturing)
        return true;

    return is_winning_capture(board, move, side);
}

bool is_winning_capture(Board *board, MOVE move, int side)
{
    int v = board->static_value(side);

    MoveType mt;
    if (!board->move(move, &mt))
        return false;
    if (mt == KING_CAPTURE)
    {
        board->unmove();
        return true;
    }

    int ret = -static_exchange_eval(board, 1 - side, move_dst(move));

    board->unmove();

    return v <= ret;
}

int static_exchange_eval(Board *board, int side, POSITION pos)
{
    int ans = board->static_value(side);

    MOVE response;
    MoveType mt;
    if (!board->is_attacked(pos, true, &response) || !board->move(response, &mt))
        return ans;

    int ret;
    if (mt == KING_CAPTURE)
        ret = INF;
    else
        ret = -static_exchange_eval(board, 1 - side, pos);

    board->unmove();

    return max(ans, ret);
}
