#include <iostream>
#include "board.h"

using namespace std;

POSITION make_position(int rank, int col)
{
    return (POSITION) ((rank << 4) | col);
}

int position_rank(POSITION p)
{
    return p >> 4;
}

int position_col(POSITION p)
{
    return p & 0xf;
}

Board::Board()
    : Board("rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR")
{
}

Board::Board(string fen)
{
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
            board[i][j].piece = make_piece(fen[k]);
            ++j;
        }
    }
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
}
