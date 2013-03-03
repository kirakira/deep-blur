#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "piece.h"
#include "move.h"
#include "rc4.h"

class Board
{
    public:
        Board();
        Board(std::string fen);

        bool move(Move m);
        bool checked_move(Move m);
        void unmove();
        bool checked_unmove();

        bool is_checked();
        uint64_t hash_code(int side);
        int static_value(int side);

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

        typedef struct sHistoryEntry
        {
            Move move;
            BoardEntry capture;
        } HistoryEntry;

        BoardEntry board[H][W];
        PieceEntry pieces[32];

        uint64_t get_hash(int rank, int col, PIECE piece);
        const uint64_t hash_side = rc4_uint64[H * W * 16];
        uint64_t hash;

        static const int static_values[16][H][W];
        int current_static_value;

        std::vector<HistoryEntry> history;
};
