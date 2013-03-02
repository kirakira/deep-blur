#include "piece.h"

PIECE make_piece(int side, int type)
{
    return (side << 3) | type;
}

PIECE make_piece(char letter)
{
    int side = 0;
    if (letter >= 'A' && letter <= 'Z')
    {
        side = 1;
        letter += ('a' - 'A');
    }

    int type;
    if (letter == 'k')
        type = PIECE_K;
    else if (letter == 'a')
        type = PIECE_A;
    else if (letter == 'e')
        type = PIECE_E;
    else if (letter == 'h')
        type = PIECE_H;
    else if (letter == 'r')
        type = PIECE_R;
    else if (letter == 'c')
        type = PIECE_C;
    else if (letter == 'p')
        type = PIECE_P;
    else
        return 0;

    return make_piece(side, type);
}

int piece_side(PIECE piece)
{
    return piece >> 3;
}

int piece_type(PIECE piece)
{
    return piece & 7;
}

char piece_letter(PIECE piece)
{
    const char array[17] = ".kaehrcp?KAEHRCP";
    return array[piece];
}
