#include "hash.h"

HashSet::HashSet(int depth)
    : containsZero(false)
{
    table = new uint64_t[1 << depth]();
    mask = (1 << depth) - 1;
}

HashSet::~HashSet()
{
    delete[] table;
}

int HashSet::hash(uint64_t value)
{
    return (int) value & mask;
}

void HashSet::put(uint64_t value)
{
    if (value == 0)
    {
        containsZero = true;
        return;
    }

    int index = hash(value), i = index;
    do
    {
        if (table[i] == 0)
        {
            table[i] = value;
            return;
        }
        else
            i = (i + 1) & mask;
    } while (i != index);

    int old_size = mask + 1;
    int new_size = (mask + 1) * 2;
    mask = new_size - 1;
    uint64_t *tmp = table;
    table = new uint64_t[new_size]();
    for (int i = 0; i < old_size; ++i)
        put(tmp[i]);
    delete []tmp;

    put(value);
}

bool HashSet::contains(uint64_t value)
{
    if (value == 0)
        return containsZero;
    int index = hash(value), i = index;
    do
    {
        if (table[i] == 0)
            return false;
        else if (table[i] == value)
            return true;
        else
            i = (i + 1) & mask;
    } while (i != index);
    return false;
}

void HashSet::remove(uint64_t value)
{
    if (value == 0)
    {
        containsZero = false;
        return;
    }

    int index = hash(value), i = index;
    int pos = -1;
    do
    {
        if (table[i] == 0)
            break;
        else if (table[i] == value)
        {
            pos = i;
            break;
        }
        else
            i = (i + 1) & mask;
    } while (i != index);
    if (pos == -1)
        return;

    table[pos] = 0;
    i = (pos + 1) & mask;
    int hole = pos;
    while (i != pos)
    {
        if (table[i] == 0)
            break;

        bool b1 = hole < i;
        int h = hash(table[i]);
        bool b2 = (h > hole && h <= i);
        if ((b1 || b2) && (!b1 || !b2))
        {
            table[hole] = table[i];
            table[i] = 0;
            hole = i;
        }

        i = (i + 1) & mask;
    }
}
