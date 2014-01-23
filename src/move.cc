#include "move.h"

using namespace std;

POSITION make_position(int rank, int col)
{
    return (POSITION) ((rank << 4) | col);
}

POSITION make_position(string s)
{
    if (s.length() < 2)
        return 0;
    else
    {
        int col = s[0] - 'a', rank = 9 - (s[1] - '0');
        return make_position(rank, col);
    }
}

int position_rank(POSITION p)
{
    return p >> 4;
}

int position_col(POSITION p)
{
    return p & 0xf;
}

string position_string(POSITION p)
{
    string ret;
    int rank = position_rank(p), col = position_col(p);
    ret += (char) (col + 'a');
    ret += (char) (9 - rank + '0');
    return ret;
}

Move::Move(POSITION source, POSITION destination)
    : src(source)
    , dst(destination)
{
}

Move::Move(string s)
{
    if (s.length() >= 4)
    {
        src = make_position(s.substr(0, 2));
        dst = make_position(s.substr(2, 2));
    }
    else
    {
        src = 0;
        dst = 0;
    }
}

string Move::to_string()
{
    return position_string(src) + position_string(dst);
}
