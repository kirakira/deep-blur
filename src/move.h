#pragma once

#include <string>
#include <stdint.h>

typedef uint8_t POSITION;
typedef uint16_t MOVE;

POSITION make_position(int rank, int col);
POSITION make_position(std::string s);
int position_rank(POSITION p);
int position_col(POSITION p);
std::string position_string(POSITION p);

MOVE make_move(POSITION src, POSITION dst);
MOVE make_move(std::string s);
POSITION move_src(MOVE move);
POSITION move_dst(MOVE move);
std::string move_string(MOVE move);
