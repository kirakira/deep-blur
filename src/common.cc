#include "common.h"

void catPV(PV *main, PV *cat)
{
    for (int i = 0; i < cat->count; ++i)
        main->moves[main->count + i] = cat->moves[i];
    main->count += cat->count;
}
