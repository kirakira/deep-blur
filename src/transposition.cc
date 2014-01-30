#include "transposition.h"

Transposition::Transposition(int table_depth)
    : t_depth(table_depth)
    , table(NULL)
{
    if (t_depth < 10)
        t_depth = 10;
    mask = ((((uint64_t) 1) << t_depth) - 1);
    rev_mask = ~mask;
    clear();
}

Transposition::~Transposition()
{
    if (table)
    {
        delete[] table;
        table = NULL;
    }
}

void Transposition::clear()
{
    if (table)
        delete[] table;
    table = new TranspositionEntry[1 << t_depth]();
}

void Transposition::put(uint64_t key, int score, int exact, MOVE move, int depth)
{
    int index = (int) (key & mask);
    table[index].key = (key & rev_mask) | (depth << 2) | exact;
    table[index].value = (score << 16) | move;
}

bool Transposition::get(uint64_t key, int *score, int *exact, MOVE *move, int *depth)
{
    int index = (int) (key & mask);
    if (table[index].key == 0 || (table[index].key & rev_mask) != (key & rev_mask))
        return false;
    int t = table[index].key & mask;
    *depth = t >> 2;
    *exact = t & 3;
    *score = table[index].value >> 16;
    *move = table[index].value & 0xffff;
    return true;
}
