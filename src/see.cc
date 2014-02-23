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

    bool game_end;
    if (!board->move(move, &game_end))
        return false;
    if (game_end)
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
    bool game_end;
    if (!board->is_attacked(pos, true, &response) || !board->move(response, &game_end))
        return ans;

    int ret;
    if (game_end)
        ret = INF;
    else
        ret = -static_exchange_eval(board, 1 - side, pos);

    board->unmove();

    return max(ans, ret);
}
