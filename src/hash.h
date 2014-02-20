#pragma once

#include <stdint.h>

class HashSet
{
    private:
        HashSet &operator=(const HashSet &);
        HashSet(const HashSet &);

        uint64_t *table;
        int mask;
        bool containsZero;

        int hash(uint64_t value);

    public:
        HashSet(int depth);
        ~HashSet();

        void put(uint64_t value);
        void remove(uint64_t value);
        bool contains(uint64_t value);
};
