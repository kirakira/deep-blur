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
    used = 0;
}

void Transposition::stat()
{
    int tot = (1 << t_depth);
    cout << "# transposition collision rate: " << collision << "/" << access << " (" << (double) collision * 100 / (double) access << "%)" << endl;
    cout << "# transposition usage: " << used << " out of " << tot << ", "
       << (double) used * 100 / (double) tot << "%" << endl;
}
