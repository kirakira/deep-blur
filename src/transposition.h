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

        void put(uint64_t key, int score, int exact, MOVE move, int depth);
        bool get(uint64_t key, int *score, int *exact, MOVE *move, int *depth);

        void clear();

        void stat();

        static const int EXACT = 1, UPPER = 2, LOWER = 3;
};
