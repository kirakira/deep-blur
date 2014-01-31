#include <iostream>

#include "transposition.h"

using namespace std;

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
    access = 0;
    collision = 0;
}

void Transposition::put(uint64_t key, int score, int exact, MOVE move, int depth)
{
    int index = (int) (key & mask);
    table[index].key = (key & rev_mask) | (depth << 2) | exact;
    table[index].value = (score << 16) | move;
}

bool Transposition::get(uint64_t key, int *score, int *exact, MOVE *move, int *depth)
{
    ++access;
    int index = (int) (key & mask);
    if (table[index].key == 0 || (table[index].key & rev_mask) != (key & rev_mask))
    {
        if (table[index].key != 0)
            ++collision;
        return false;
    }
    int t = table[index].key & mask;
    *depth = t >> 2;
    *exact = t & 3;
    *score = table[index].value >> 16;
    *move = table[index].value & 0xffff;
    return true;
}

void Transposition::stat()
{
    int tot = (1 << t_depth), occ = 0;
    for (int i = 0; i < (1 << t_depth); ++i)
        if (table[i].key != 0)
            ++occ;
    cout << "transposition collision rate: " << collision << "/" << access << " (" << (double) collision * 100 / (double) access << "%)" << endl;
    cout << "transposition usage: " << occ << " out of " << tot << ", "
       << (double) 100 * occ / (double) tot << "%" << endl;
}
