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

    int starting_position[16] =
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
            int index = starting_position[piece];
            ++starting_position[piece];

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
    if (!(src_i >= 0 && src_i < H && dst_i >= 0 && dst_i < H
            && src_j >= 0 && src_j < W && dst_j >= 0 && dst_j < W))
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
