#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "piece.h"
#include "move.h"

class Board
{
    public:
        Board(std::string fen = "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR");

        void set(std::string fen);

        bool move(MOVE m, bool *game_end = NULL);
        bool checked_move(MOVE m);
        void unmove();
        bool checked_unmove();

        bool in_check(int side);
        bool is_capture(MOVE move, int *value = NULL);
        POSITION king_position(int side);

        uint64_t hash_code(int side);
        int static_value(int side);

        void generate_moves(int side, MOVE *moves, int *capture_scores, int *moves_count);

        void print();

        static const int H = 10, W = 9, NON_CAPTURE = -127;

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
            MOVE move;
            BoardEntry capture;
        } HistoryEntry;

        BoardEntry board[H][W];
        PieceEntry pieces[32];

        uint64_t get_hash(int rank, int col, PIECE piece);
        const uint64_t hash_side;
        uint64_t hash;

        static const int static_values[16][H][W];
        static const int capture_values[8];
        int current_static_value;

        std::vector<HistoryEntry> history;

        static bool is_valid_position(PIECE piece, int i, int j);
        static bool is_in_palace(int side, int i, int j);
        static bool is_in_half(int side, int i, int j);
        static bool is_on_board(int i, int j);
        bool check_position(int side, int i, int j, int *target_capture_score);

        class BoardStaticFieldsInitializer
        {
            public:
                BoardStaticFieldsInitializer();
        };
        static BoardStaticFieldsInitializer board_initializer;

        static int c4di[4], c4dj[4];
        static int king_moves[256][4][2], king_moves_count[256];
        static int horse_d[8][4];
        static int horse_moves[256][8][4], horse_moves_count[256];
        static int s4di[4], s4dj[4];
        static int elephant_positions[7][2];
        static int elephant_moves[256][4][4], elephant_moves_count[256];
        static int assistant_moves[256][4][2], assistant_moves_count[256];
        static int pawn_moves[2][256][3][2], pawn_moves_count[2][256];

        void add_move(MOVE *moves, int *capture_scores, int *moves_count, MOVE move_to_add, int capture_score);
        void generate_king_moves(int index, MOVE *moves, int *capture_scores, int *moves_count);
        void generate_rook_moves(int index, MOVE *moves, int *capture_scores, int *moves_count);
        void generate_horse_moves(int index, MOVE *moves, int *capture_scores, int *moves_count);
        void generate_cannon_moves(int index, MOVE *moves, int *capture_scores, int *moves_count);
        void generate_elephant_moves(int index, MOVE *moves, int *capture_scores, int *moves_count);
        void generate_assistant_moves(int index, MOVE *moves, int *capture_scores, int *moves_count);
        void generate_pawn_moves(int index, MOVE *moves, int *capture_scores, int *moves_count);
};
