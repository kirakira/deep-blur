#include "common.h"

#include <sstream>
#include <iomanip>

using namespace std;

void catPV(PV *main, PV *cat)
{
    for (int i = 0; i < cat->count; ++i)
        main->moves[main->count + i] = cat->moves[i];
    main->count += cat->count;
}

string readable_number(int x)
{
    ostringstream oss;
    if (x < 1000)
        oss << x;
    else if (x < 1000000)
        oss << setprecision(2) << fixed << (double) x / 1000 << "k";
    else
        oss << setprecision(2) << fixed << (double) x / 1000000 << "m";
    return oss.str();
}
