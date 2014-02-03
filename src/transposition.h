#pragma once

#include <stdint.h>
#include "move.h"

class Transposition
{
    private:
        Transposition(const Transposition &);
        Transposition &operator=(const Transposition &);
        bool operator==(const Transposition &);

    protected:
        typedef struct sTranspositionEntry
        {
            uint64_t key;
            int32_t value;
        } TranspositionEntry;

        uint64_t mask, rev_mask;
        int t_depth;
        TranspositionEntry *table;

        int used, access, collision;

    public:
        // table_depth should be at least 10
        Transposition(int table_depth);
        ~Transposition();

        inline void put(uint64_t key, int score, int exact, MOVE move, int depth)
        {
            int index = (int) (key & mask);
            if (table[index].key == 0)
                ++used;
            table[index].key = (key & rev_mask) | (depth << 2) | exact;
            table[index].value = (score << 16) | move;
        }


        inline bool get(uint64_t key, int *score, int *exact, MOVE *move, int *depth)
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


        void clear();

        void stat();

        static const int EXACT = 1, UPPER = 2, LOWER = 3;
};
