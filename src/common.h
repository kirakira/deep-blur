#pragma once

#include "move.h"

static const int INF = 2047;
static const int MAX_DEPTH = 80;

typedef struct sPV
{
    MOVE moves[MAX_DEPTH + 1];
    int count;
} PV;

void catPV(PV *main, PV *cat);
