#pragma once

#include <string>

typedef uint8_t POSITION;

POSITION make_position(int rank, int col);
POSITION make_position(std::string s);
int position_rank(POSITION p);
int position_col(POSITION p);
std::string position_string(POSITION p);

class Move
{
public:
    POSITION src, dst;

    Move();
    Move(POSITION source, POSITION destination);
    Move(std::string s);

    std::string to_string();
};
