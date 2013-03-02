#pragma once

#include <cstdint>
#include <string>

#include "piece.h"
#include "rc4.h"

typedef uint8_t POSITION;

POSITION make_position(int rank, int col);
int position_rank(POSITION p);
int position_col(POSITION p);

typedef struct sMove
{
    POSITION src, dst;
} Move;


class Board
{
    public:
        Board();
        Board(std::string fen);

        bool move(Move m);
        void unmove();
        bool is_checked();
        uint64_t hash_code(int side);

        void print();

        static const int H = 10, W = 9;

    protected:
        typedef struct sBoardEntry
        {
            uint8_t index;
            PIECE piece;
        } BoardEntry;

        typedef struct sPieceEntry
        {
            POSITION position;
            PIECE piece;
        } PieceEntry;

        BoardEntry board[H][W];
        PieceEntry pieces[32];

        uint64_t get_hash(int rank, int col, PIECE piece);
        const uint64_t hash_side = rc4_uint64[W * H * 16];
        uint64_t hash;
};
