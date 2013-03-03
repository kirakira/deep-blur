#include <iostream>
#include <cstring>
#include "board.h"

using namespace std;

Board::Board()
    : Board("rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR")
{
}

Board::Board(string fen)
{
    for (int i = 0; i < 32; ++i)
    {
        pieces[i].position = 0;
        pieces[i].piece = 0;
    }

    int start_position[16] =
        {0, 0, 1, 3, 5, 7, 9, 11,
         16, 16, 17, 19, 21, 23, 25, 27};

    hash = 0;
    current_static_value = 0;

    int i = 0, j = 0;
    for (size_t k = 0; k < fen.length(); ++k)
    {
        if (fen[k] == '/')
        {
            ++i;
            j = 0;
        }
        else if (fen[k] >= '0' && fen[k] <= '9')
        {
            int count = fen[k] - '0';
            while (count > 0)
            {
                board[i][j].index = 0;
                board[i][j].piece = 0;
                ++j;
                --count;
            }
        }
        else
        {
            PIECE piece = make_piece(fen[k]);
            int index = start_position[piece];
            ++start_position[piece];

            board[i][j].index = index;
            board[i][j].piece = piece;
            pieces[index].position = make_position(i, j);
            pieces[index].piece = piece;

            hash ^= get_hash(i, j, piece);
            current_static_value += static_values[piece][i][j];

            ++j;
        }
    }

    history.reserve(128);
}

void Board::print()
{
    cout << "   a b c d e f g h i" << endl << endl;
    for (int i = 0; i < H; ++i)
    {
        cout << H - 1 - i << "  ";
        for (int j = 0; j < W; ++j)
        {
            if (board[i][j].piece == 0)
                cout << '.';
            else
                cout << piece_letter(board[i][j].piece);
            cout << " ";
        }
        cout << " " << H - 1 - i << endl;
    }
    cout << endl << "   a b c d e f g h i" << endl;
    cout << "Hash code: " << hash_code(0) << ", " << hash_code(1) <<  endl;
    cout << "Static value: " << static_value(0) << endl;
}

uint64_t Board::get_hash(int rank, int col, PIECE piece)
{
    return rc4_uint64[rank * W * 16 + col * 16 + piece];
}

uint64_t Board::hash_code(int side)
{
    if (side == 0)
        return hash;
    else
        return hash ^ hash_side;
}

bool Board::checked_move(Move move)
{
    int src_i = position_rank(move.src),
        src_j = position_col(move.src),
        dst_i = position_rank(move.dst),
        dst_j = position_col(move.dst);
    if (!(is_on_board(src_i, src_j) && is_on_board(src_i, src_j)))
        return false;
    if (board[src_i][src_j].piece == 0)
        return false;
    if (src_i == dst_i && src_j == dst_j)
        return false;
    return this->move(move);
}

bool Board::move(Move move)
{
    int src_i = position_rank(move.src),
        src_j = position_col(move.src),
        dst_i = position_rank(move.dst),
        dst_j = position_col(move.dst);
    BoardEntry src = board[src_i][src_j],
               dst = board[dst_i][dst_j];

    if (dst.piece != 0)
    {
        pieces[dst.index].piece = 0;
        hash ^= get_hash(dst_i, dst_j, dst.piece);
        current_static_value -= static_values[dst.piece][dst_i][dst_j];
    }

    board[dst_i][dst_j] = src;
    pieces[src.index].position = make_position(dst_i, dst_j);
    hash ^= get_hash(dst_i, dst_j, src.piece);
    current_static_value += static_values[src.piece][dst_i][dst_j];

    board[src_i][src_j].piece = 0;
    hash ^= get_hash(src_i, src_j, src.piece);
    current_static_value -= static_values[src.piece][src_i][src_j];

    HistoryEntry history_entry;
    history_entry.move = move;
    history_entry.capture = dst;
    history.push_back(history_entry);

    return true;
}

void Board::unmove()
{
    HistoryEntry history_entry = history.back();
    history.pop_back();

    int src_i = position_rank(history_entry.move.src),
        src_j = position_col(history_entry.move.src),
        dst_i = position_rank(history_entry.move.dst),
        dst_j = position_col(history_entry.move.dst);

    BoardEntry src = board[dst_i][dst_j], dst = history_entry.capture;

    board[src_i][src_j] = src;
    pieces[src.index].position = make_position(src_i, src_j);
    hash ^= get_hash(src_i, src_j, src.piece);
    current_static_value += static_values[src.piece][src_i][src_j];

    hash ^= get_hash(dst_i, dst_j, src.piece);
    current_static_value -= static_values[src.piece][dst_i][dst_j];
    board[dst_i][dst_j] = dst;
    if (dst.piece != 0)
    {
        pieces[dst.index].piece = dst.piece;
        pieces[dst.index].position = make_position(dst_i, dst_j);
        hash ^= get_hash(dst_i, dst_j, dst.piece);
        current_static_value += static_values[dst.piece][dst_i][dst_j];
    }
}

bool Board::checked_unmove()
{
    if (history.size() == 0)
        return false;
    unmove();
    return true;
}

const int Board::static_values[16][H][W] =
    {{{0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 0, 11, 15, 11, 0, 0, 0},
    {0, 0, 0, 2, 2, 2, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 0, 20, 0, 20, 0, 0, 0},
    {0, 0, 0, 0, 23, 0, 0, 0, 0},
    {0, 0, 0, 20, 0, 20, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 20, 0, 0, 0, 20, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {18, 0, 0, 0, 23, 0, 0, 0, 18},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 20, 0, 0, 0, 20, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {{88, 85, 90, 88, 90, 88, 90, 85, 88},
    {85, 90, 92, 93, 78, 93, 92, 90, 85},
    {93, 92, 94, 95, 92, 95, 94, 92, 93},
    {92, 94, 98, 95, 98, 95, 98, 94, 92},
    {90, 98, 101, 102, 103, 102, 101, 98, 90},
    {90, 100, 99, 103, 104, 103, 99, 100, 90},
    {93, 108, 100, 107, 100, 107, 100, 108, 93},
    {92, 98, 99, 103, 99, 103, 99, 98, 92},
    {90, 96, 103, 97, 94, 97, 103, 96, 90},
    {90, 90, 90, 96, 90, 96, 90, 90, 90}},
    {{194, 206, 204, 212, 200, 212, 204, 206, 194},
    {200, 208, 206, 212, 200, 212, 206, 208, 200},
    {198, 208, 204, 212, 212, 212, 204, 208, 198},
    {204, 209, 204, 212, 214, 212, 204, 209, 204},
    {208, 212, 212, 214, 215, 214, 212, 212, 208},
    {208, 211, 211, 214, 215, 214, 211, 211, 208},
    {206, 213, 213, 216, 216, 216, 213, 213, 206},
    {206, 208, 207, 214, 216, 214, 207, 208, 206},
    {206, 212, 209, 216, 233, 216, 209, 212, 206},
    {206, 208, 207, 213, 214, 213, 207, 208, 206}},
    {{96, 96, 97, 99, 99, 99, 97, 96, 96},
    {96, 97, 98, 98, 98, 98, 98, 97, 96},
    {97, 96, 100, 99, 101, 99, 100, 96, 97},
    {96, 96, 96, 96, 96, 96, 96, 96, 96},
    {95, 96, 99, 96, 100, 96, 99, 96, 95},
    {96, 96, 96, 96, 100, 96, 96, 96, 96},
    {96, 99, 99, 98, 100, 98, 99, 99, 96},
    {97, 97, 96, 91, 92, 91, 96, 97, 97},
    {98, 98, 96, 92, 89, 92, 96, 98, 98},
    {100, 100, 96, 91, 90, 91, 96, 100, 100}},
    {{0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {7, 0, 7, 0, 15, 0, 7, 0, 7},
    {7, 0, 13, 0, 16, 0, 13, 0, 7},
    {14, 18, 20, 27, 29, 27, 20, 18, 14},
    {19, 23, 27, 29, 30, 29, 27, 23, 19},
    {19, 24, 32, 37, 37, 37, 32, 24, 19},
    {19, 24, 34, 42, 44, 42, 34, 24, 19},
    {9, 9, 9, 11, 13, 11, 9, 9, 9}},
    {{0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, -1, -1, -1, 0, 0, 0},
    {0, 0, 0, -2, -2, -2, 0, 0, 0},
    {0, 0, 0, -11, -15, -11, 0, 0, 0}},
    {{0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, -20, 0, -20, 0, 0, 0},
    {0, 0, 0, 0, -23, 0, 0, 0, 0},
    {0, 0, 0, -20, 0, -20, 0, 0, 0}},
    {{0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, -20, 0, 0, 0, -20, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {-18, 0, 0, 0, -23, 0, 0, 0, -18},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, -20, 0, 0, 0, -20, 0, 0}},
    {{-90, -90, -90, -96, -90, -96, -90, -90, -90},
    {-90, -96, -103, -97, -94, -97, -103, -96, -90},
    {-92, -98, -99, -103, -99, -103, -99, -98, -92},
    {-93, -108, -100, -107, -100, -107, -100, -108, -93},
    {-90, -100, -99, -103, -104, -103, -99, -100, -90},
    {-90, -98, -101, -102, -103, -102, -101, -98, -90},
    {-92, -94, -98, -95, -98, -95, -98, -94, -92},
    {-93, -92, -94, -95, -92, -95, -94, -92, -93},
    {-85, -90, -92, -93, -78, -93, -92, -90, -85},
    {-88, -85, -90, -88, -90, -88, -90, -85, -88}},
    {{-206, -208, -207, -213, -214, -213, -207, -208, -206},
    {-206, -212, -209, -216, -233, -216, -209, -212, -206},
    {-206, -208, -207, -214, -216, -214, -207, -208, -206},
    {-206, -213, -213, -216, -216, -216, -213, -213, -206},
    {-208, -211, -211, -214, -215, -214, -211, -211, -208},
    {-208, -212, -212, -214, -215, -214, -212, -212, -208},
    {-204, -209, -204, -212, -214, -212, -204, -209, -204},
    {-198, -208, -204, -212, -212, -212, -204, -208, -198},
    {-200, -208, -206, -212, -200, -212, -206, -208, -200},
    {-194, -206, -204, -212, -200, -212, -204, -206, -194}},
    {{-100, -100, -96, -91, -90, -91, -96, -100, -100},
    {-98, -98, -96, -92, -89, -92, -96, -98, -98},
    {-97, -97, -96, -91, -92, -91, -96, -97, -97},
    {-96, -99, -99, -98, -100, -98, -99, -99, -96},
    {-96, -96, -96, -96, -100, -96, -96, -96, -96},
    {-95, -96, -99, -96, -100, -96, -99, -96, -95},
    {-96, -96, -96, -96, -96, -96, -96, -96, -96},
    {-97, -96, -100, -99, -101, -99, -100, -96, -97},
    {-96, -97, -98, -98, -98, -98, -98, -97, -96},
    {-96, -96, -97, -99, -99, -99, -97, -96, -96}},
    {{-9, -9, -9, -11, -13, -11, -9, -9, -9},
    {-19, -24, -34, -42, -44, -42, -34, -24, -19},
    {-19, -24, -32, -37, -37, -37, -32, -24, -19},
    {-19, -23, -27, -29, -30, -29, -27, -23, -19},
    {-14, -18, -20, -27, -29, -27, -20, -18, -14},
    {-7, 0, -13, 0, -16, 0, -13, 0, -7},
    {-7, 0, -7, 0, -15, 0, -7, 0, -7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}}};

int Board::static_value(int side)
{
    if (side == 0)
        return current_static_value;
    else
        return -current_static_value;
}

bool Board::is_valid_position(PIECE piece, int i, int j)
{
    return (static_values[piece][i][j] != 0);
}

bool Board::is_on_board(int i, int j)
{
    return (i >= 0 && i < H && j >= 0 && j < W);
}

bool Board::is_in_palace(int side, int i, int j)
{
    if (side == 0)
        return (i >= 0 && i <= 2 && j >= 3 && j <= 5);
    else
        return (i >= 7 && i <= 9 && j >= 3 && j <= 5);
}

bool Board::check_position(int side, int i, int j)
{
    if (board[i][j].piece == 0 || piece_side(board[i][j].piece) != side)
        return true;
    else
        return false;
}

int Board::c4di[4] = {0, 1, 0, -1}, Board::c4dj[4] = {1, 0, -1, 0};
int Board::king_moves[256][4][2], Board::king_moves_count[256];
int Board::horse_d[8][4] = {
    {-2, 1, -1, 0},
    {-1, 2, 0, 1},
    {1, 2, 0, 1},
    {2, 1, 1, 0},
    {2, -1, 1, 0},
    {1, -2, 0, -1},
    {-1, -2, 0, -1},
    {-2, -1, -1, 0}
};
int Board::horse_moves[256][8][4], Board::horse_moves_count[256];
Board::BoardStaticFieldsInitializer Board::board_initializer;

Board::BoardStaticFieldsInitializer::BoardStaticFieldsInitializer()
{
    // King
    for (int side = 0; side <= 1; ++side)
    {
        for (int ii = 0; ii <= 2; ++ii)
        {
            int i = ii;
            if (side == 1)
                i = 9 - ii;
            for (int j = 3; j <= 5; ++j)
            {
                POSITION p = make_position(i, j);
                king_moves_count[p] = 0;
                for (int r = 0; r < 4; ++r)
                {
                    int oi = i + c4di[r], oj = j + c4dj[r];
                    if (is_in_palace(side, oi, oj))
                    {
                        int index = king_moves_count[p];
                        ++king_moves_count[p];

                        king_moves[p][index][0] = oi;
                        king_moves[p][index][1] = oj;
                    }
                }
            }
        }
    }

    // Horse
    for (int i = 0; i < H; ++i)
        for (int j = 0; j < W; ++j)
        {
            POSITION p = make_position(i, j);
            horse_moves_count[p] = 0;
            for (int r = 0; r < 8; ++r)
            {
                int oi = i + horse_d[r][0], oj = j + horse_d[r][1];
                if (is_on_board(oi, oj))
                {
                    int index = horse_moves_count[p];
                    ++horse_moves_count[p];

                    horse_moves[p][index][0] = oi;
                    horse_moves[p][index][1] = oj;
                    horse_moves[p][index][2] = i + horse_d[r][2];
                    horse_moves[p][index][3] = j + horse_d[r][3];
                }
            }
        }
}

void Board::add_move(Move *moves, int *moves_count, Move move_to_add)
{
    moves[*moves_count] = move_to_add;
    ++*moves_count;
}

void Board::generate_moves(int side, Move *moves, int *moves_count)
{
    int index;
    *moves_count = 0;

    // Rook
    index = 7;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 2; ++i)
        if (pieces[index + i].piece != 0)
            generate_rook_moves(index + i, moves, moves_count);

    // Horse
    index = 5;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 2; ++i)
        if (pieces[index + i].piece != 0)
            generate_horse_moves(index + i, moves, moves_count);

    // King
    index = 0;
    if (side != 0)
        index += 16;
    generate_king_moves(index, moves, moves_count);
}

void Board::generate_king_moves(int index, Move *moves, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);
    for (int i = 0; i < king_moves_count[pos]; ++i)
    {
        int oi = king_moves[pos][i][0], oj = king_moves[pos][i][1];
        if (check_position(side, oi, oj))
            add_move(moves, moves_count, Move(pos, make_position(oi, oj)));
    }
}

void Board::generate_rook_moves(int index, Move *moves, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);

    int i = position_rank(pos), j = position_col(pos);
    for (int r = 0; r < 4; ++r)
    {
        int oi = i, oj = j;
        while (true)
        {
            oi += c4di[r];
            oj += c4dj[r];

            if (!is_on_board(oi, oj))
                break;

            if (board[oi][oj].piece == 0)
                add_move(moves, moves_count, Move(pos, make_position(oi, oj)));
            else
            {
                if (check_position(side, oi, oj))
                    add_move(moves, moves_count, Move(pos, make_position(oi, oj)));
                break;
            }
        }
    }
}

void Board::generate_horse_moves(int index, Move *moves, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);

    for (int i = 0; i < horse_moves_count[pos]; ++i)
    {
        int oi = horse_moves[pos][i][0], oj = horse_moves[pos][i][1];
        if (check_position(side, oi, oj) &&
                board[horse_moves[pos][i][2]][horse_moves[pos][i][3]].piece == 0)
            add_move(moves, moves_count, Move(pos, make_position(oi, oj)));
    }
}
