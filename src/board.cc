#include <iostream>
#include <cstring>
#include "board.h"
#include "rc4.h"

using namespace std;

Board::Board(string fen)
    : hash_side(rc4_uint64[H * W * 16])
{
    set(fen);
}

void Board::set(string fen)
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

    history.clear();
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

bool Board::checked_move(MOVE move)
{
    int src_i = position_rank(move_src(move)),
        src_j = position_col(move_src(move)),
        dst_i = position_rank(move_dst(move)),
        dst_j = position_col(move_dst(move));
    if (!(is_on_board(src_i, src_j) && is_on_board(dst_i, dst_j)))
        return false;
    if (board[src_i][src_j].piece == 0)
        return false;
    if (src_i == dst_i && src_j == dst_j)
        return false;
    return this->move(move);
}

bool Board::move(MOVE move, bool *game_end)
{
    int src_i = position_rank(move_src(move)),
        src_j = position_col(move_src(move)),
        dst_i = position_rank(move_dst(move)),
        dst_j = position_col(move_dst(move));
    BoardEntry src = board[src_i][src_j],
               dst = board[dst_i][dst_j];

    if (game_end)
        *game_end = false;

    if (dst.piece != 0)
    {
        if (piece_type(dst.piece) == PIECE_K && game_end)
            *game_end = true;
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

    int src_i = position_rank(move_src(history_entry.move)),
        src_j = position_col(move_src(history_entry.move)),
        dst_i = position_rank(move_dst(history_entry.move)),
        dst_j = position_col(move_dst(history_entry.move));

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

bool Board::is_capture(MOVE move, int *value)
{
    int src_i = position_rank(move_src(move)),
        src_j = position_col(move_src(move)),
        dst_i = position_rank(move_dst(move)),
        dst_j = position_col(move_dst(move));
    BoardEntry src = board[src_i][src_j],
               dst = board[dst_i][dst_j];
    if (dst.piece != 0)
    {
        if (value)
            *value = capture_values[piece_type(dst.piece)] -
                capture_values[piece_type(src.piece)];
        return true;
    }
    return false;
}

POSITION Board::king_position(int side)
{
    int index = 0;
    if (side == 1)
        index += 16;
    return pieces[index].position;
}

const int Board::capture_values[8] = {0, 5, 2, 2, 3, 4, 3, 1};
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

bool Board::is_in_half(int side, int i, int j)
{
    if (!(i >= 0 && i < H && j >= 0 && j < W))
        return false;
    if ((side == 0 && i <= 4) || (side == 1 && j >= 5))
        return true;
    else
        return false;
}

bool Board::check_position(int side, int i, int j, int *target_capture_score)
{
    if (board[i][j].piece == 0)
    {
        *target_capture_score = NON_CAPTURE;
        return true;
    }
    else if (piece_side(board[i][j].piece) != side)
    {
        *target_capture_score = capture_values[piece_type(board[i][j].piece)];
        return true;
    }
    else
        return false;
}

int Board::c4di[4] = {0, 1, 0, -1}, Board::c4dj[4] = {1, 0, -1, 0};
int Board::king_moves[256][4][2], Board::king_moves_count[256] = {0};
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
int Board::s4di[4] = {1, 1, -1, -1}, Board::s4dj[4] = {-1, 1, 1, -1};
int Board::elephant_positions[7][2] = {{0, 2}, {0, 6}, {2, 0}, {2, 4}, {2, 8}, {4, 2}, {4, 6}};
int Board::elephant_moves[256][4][4], Board::elephant_moves_count[256];
int Board::assistant_moves[256][4][2], Board::assistant_moves_count[256];
int Board::pawn_moves[2][256][3][2], Board::pawn_moves_count[2][256];

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
                i = H - 1 - ii;
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

    // Assistant
    for (int side = 0; side <= 1; ++side)
    {
        for (int ii = 0; ii <= 2; ++ii)
        {
            int i = ii;
            if (side == 1)
                i = H - 1 - ii;
            for (int j = 3; j <= 5; ++j)
            {
                POSITION p = make_position(i, j);
                assistant_moves_count[p] = 0;
                for (int r = 0; r < 4; ++r)
                {
                    int oi = i + s4di[r], oj = j + s4dj[r];
                    if (is_in_palace(side, oi, oj))
                    {
                        int index = assistant_moves_count[p];
                        ++assistant_moves_count[p];

                        assistant_moves[p][index][0] = oi;
                        assistant_moves[p][index][1] = oj;
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

    // Elephant
    for (int k = 0; k < 7; ++k)
    {
        int i = elephant_positions[k][0], j = elephant_positions[k][1];
        POSITION p = make_position(i, j);
        elephant_moves_count[p] = 0;
        for (int r = 0; r < 4; ++r)
        {
            int oi = i + s4di[r] * 2, oj = j + s4dj[r] * 2;
            if (is_in_half(0, oi, oj))
            {
                int index = elephant_moves_count[p];
                ++elephant_moves_count[p];

                elephant_moves[p][index][0] = oi;
                elephant_moves[p][index][1] = oj;
                elephant_moves[p][index][2] = i + s4di[r];
                elephant_moves[p][index][3] = j + s4dj[r];
            }
        }

        int pi = H - 1 - i, pj = j;
        POSITION pp = make_position(pi, pj);
        elephant_moves_count[pp] = elephant_moves_count[p];
        for (int kk = 0; kk < elephant_moves_count[p]; ++kk)
        {
            elephant_moves[pp][kk][0] = H - 1 - elephant_moves[p][kk][0];
            elephant_moves[pp][kk][1] = elephant_moves[p][kk][1];
            elephant_moves[pp][kk][2] = H - 1 - elephant_moves[p][kk][2];
            elephant_moves[pp][kk][3] = elephant_moves[p][kk][3];
        }
    }

    // Pawn
    for (int i = 3; i <= 4; ++i)
        for (int j = 0; j < W; j += 2)
        {
            POSITION p = make_position(i, j);
            pawn_moves_count[0][p] = 1;
            pawn_moves[0][p][0][0] = i + 1;
            pawn_moves[0][p][0][1] = j;
        }
    for (int i = 5; i < H; ++i)
        for (int j = 0; j < W; ++j)
        {
            POSITION p = make_position(i, j);
            pawn_moves_count[0][p] = 0;
            for (int r = 0; r < 4; ++r)
            {
                int oi = i + c4di[r], oj = j + c4dj[r];
                if (c4di[r] >= 0 && is_on_board(oi, oj))
                {
                    int index = pawn_moves_count[0][p];
                    ++pawn_moves_count[0][p];

                    pawn_moves[0][p][index][0] = oi;
                    pawn_moves[0][p][index][1] = oj;
                }
            }
        }
    for (int i = 0; i < H; ++i)
        for (int j = 0; j < W; ++j)
        {
            int pi = H - 1 - i, pj = j;
            if (pawn_moves_count[0][make_position(pi, pj)] > 0)
            {
                pawn_moves_count[1][make_position(i, j)] = pawn_moves_count[0][make_position(pi, pj)];
                for (int r = 0; r < pawn_moves_count[1][make_position(i, j)]; ++r)
                {
                    pawn_moves[1][make_position(i, j)][r][0] = H - 1 - pawn_moves[0][make_position(pi, pj)][r][0];
                    pawn_moves[1][make_position(i, j)][r][1] = pawn_moves[0][make_position(pi, pj)][r][1];
                }
            }
        }
}

void Board::add_move(MOVE *moves, int *capture_scores, int *moves_count, MOVE move_to_add, int capture_score)
{
    moves[*moves_count] = move_to_add;
    capture_scores[*moves_count] = capture_score;
    ++*moves_count;
}

void Board::generate_moves(int side, MOVE *moves, int *capture_scores, int *moves_count)
{
    int index;
    *moves_count = 0;

    // Rook
    index = 7;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 2; ++i)
        if (pieces[index + i].piece != 0)
            generate_rook_moves(index + i, moves, capture_scores, moves_count);

    // Horse
    index = 5;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 2; ++i)
        if (pieces[index + i].piece != 0)
            generate_horse_moves(index + i, moves, capture_scores, moves_count);

    // Cannon
    index = 9;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 2; ++i)
        if (pieces[index + i].piece != 0)
            generate_cannon_moves(index + i, moves, capture_scores, moves_count);

    // Pawn
    index = 11;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 5; ++i)
        if (pieces[index + i].piece != 0)
            generate_pawn_moves(index + i, moves, capture_scores, moves_count);

    // Assistant
    index = 1;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 2; ++i)
        if (pieces[index + i].piece != 0)
            generate_assistant_moves(index + i, moves, capture_scores, moves_count);

    // Elephant
    index = 3;
    if (side != 0)
        index += 16;
    for (int i = 0; i < 2; ++i)
        if (pieces[index + i].piece != 0)
            generate_elephant_moves(index + i, moves, capture_scores, moves_count);

    // King
    index = 0;
    if (side != 0)
        index += 16;
    generate_king_moves(index, moves, capture_scores, moves_count);
}

void Board::generate_king_moves(int index, MOVE *moves, int *capture_scores, int *moves_count)
{
    POSITION pos = pieces[index].position, other_king_pos = pieces[16 - index].position;
    int side = piece_side(pieces[index].piece);
    for (int i = 0; i < king_moves_count[pos]; ++i)
    {
        int oi = king_moves[pos][i][0], oj = king_moves[pos][i][1];
        int capture_value;
        if (check_position(side, oi, oj, &capture_value))
        {
            bool face = false;
            if (oj != position_col(pos) && oj == position_col(other_king_pos))
            {
                int start, end;
                if (oi < position_rank(other_king_pos))
                {
                    start = oi + 1;
                    end = position_rank(other_king_pos) - 1;
                }
                else
                {
                    start = position_rank(other_king_pos) + 1;
                    end = oi - 1;
                }
                face = true;
                for (int j = start; face && j <= end; ++j)
                    if (board[j][oj].piece != 0)
                        face = false;
            }
            if (!face)
                add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)),
                        capture_value * 8 - capture_values[PIECE_K]);
        }
    }
}

void Board::generate_assistant_moves(int index, MOVE *moves, int *capture_values, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);
    for (int i = 0; i < assistant_moves_count[pos]; ++i)
    {
        int oi = assistant_moves[pos][i][0], oj = assistant_moves[pos][i][1];
        int capture_value;
        if (check_position(side, oi, oj, &capture_value))
            add_move(moves, capture_values, moves_count, make_move(pos, make_position(oi, oj)),
                    capture_value * 8 - capture_values[PIECE_A]);
    }
}

void Board::generate_rook_moves(int index, MOVE *moves, int *capture_scores, int *moves_count)
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
                add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)),
                        NON_CAPTURE);
            else
            {
                int capture_value;
                if (check_position(side, oi, oj, &capture_value))
                    add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)),
                            capture_value * 8 - capture_values[PIECE_R]);
                break;
            }
        }
    }
}

void Board::generate_horse_moves(int index, MOVE *moves, int *capture_scores, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);

    for (int i = 0; i < horse_moves_count[pos]; ++i)
    {
        int oi = horse_moves[pos][i][0], oj = horse_moves[pos][i][1];
        int capture_value;
        if (check_position(side, oi, oj, &capture_value) &&
                board[horse_moves[pos][i][2]][horse_moves[pos][i][3]].piece == 0)
            add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)),
                    capture_value * 8 - capture_values[PIECE_H]);
    }
}

void Board::generate_cannon_moves(int index, MOVE *moves, int *capture_scores, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);

    int i = position_rank(pos), j = position_col(pos);
    for (int r = 0; r < 4; ++r)
    {
        int oi = i, oj = j;
        int state = 0;
        while (true)
        {
            oi += c4di[r];
            oj += c4dj[r];

            if (!is_on_board(oi, oj))
                break;

            if (state == 0)
            {
                if (board[oi][oj].piece == 0)
                    add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)), NON_CAPTURE);
                else
                    state = 1;
            }
            else if (state == 1)
            {
                if (board[oi][oj].piece != 0)
                {
                    int capture_value;
                    if (check_position(side, oi, oj, &capture_value))
                        add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)),
                                capture_value * 8 - capture_values[PIECE_C]);
                    break;
                }
            }
        }
    }
}

void Board::generate_elephant_moves(int index, MOVE *moves, int *capture_scores, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);

    for (int i = 0; i < elephant_moves_count[pos]; ++i)
    {
        int oi = elephant_moves[pos][i][0], oj = elephant_moves[pos][i][1];
        int capture_value;
        if (check_position(side, oi, oj, &capture_value) &&
                board[elephant_moves[pos][i][2]][elephant_moves[pos][i][3]].piece == 0)
            add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)),
                    capture_value * 8 - capture_values[PIECE_E]);
    }
}

void Board::generate_pawn_moves(int index, MOVE *moves, int *capture_scores, int *moves_count)
{
    POSITION pos = pieces[index].position;
    int side = piece_side(pieces[index].piece);

    for (int i = 0; i < pawn_moves_count[side][pos]; ++i)
    {
        int oi = pawn_moves[side][pos][i][0], oj = pawn_moves[side][pos][i][1];
        int capture_value;
        if (check_position(side, oi, oj, &capture_value))
            add_move(moves, capture_scores, moves_count, make_move(pos, make_position(oi, oj)),
                    capture_value * 8 - capture_values[PIECE_P]);
    }
}
