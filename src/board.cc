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
    }

    board[dst_i][dst_j] = src;
    pieces[src.index].position = make_position(dst_i, dst_j);
    hash ^= get_hash(dst_i, dst_j, src.piece);

    board[src_i][src_j].piece = 0;
    hash ^= get_hash(src_i, src_j, src.piece);

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

    hash ^= get_hash(dst_i, dst_j, src.piece);
    board[dst_i][dst_j] = dst;
    if (dst.piece != 0)
    {
        pieces[dst.index].piece = dst.piece;
        pieces[dst.index].position = make_position(dst_i, dst_j);
        hash ^= get_hash(dst_i, dst_j, dst.piece);
    }
}

bool Board::checked_unmove()
{
    if (history.size() == 0)
        return false;
    unmove();
    return true;
}
