#pragma once

#include <cstdint>

typedef uint8_t PIECE;
PIECE make_piece(int side, int type);
PIECE make_piece(char letter);
int piece_side(PIECE piece);
int piece_type(PIECE piece);
char piece_letter(PIECE piece);


const int PIECE_K = 1;
const int PIECE_A = 2;
const int PIECE_E = 3;
const int PIECE_H = 4;
const int PIECE_R = 5;
const int PIECE_C = 6;
const int PIECE_P = 7;
