#include "movelist.h"
#include "see.h"

void remove_move(MOVE *moves, int *scores, int from, int &end, MOVE to_remove)
{
    for (int i = from; i < end; ++i)
        if (moves[i] == to_remove)
        {
            for (int j = i + 1; j < end; ++j)
            {
                moves[j - 1] = moves[j];
                scores[j - 1] = scores[j];
            }
            --end;
            break;
        }
}

void insert_move(MOVE *moves, int *scores, int index, int to)
{
    MOVE t = moves[index];
    int tt = scores[index];
    for (int i = index; i - 1 >= to; --i)
    {
        moves[i] = moves[i - 1];
        scores[i] = scores[i - 1];
    }
    moves[to] = t;
    scores[to] = tt;
}

int select_best(int *scores, int from, int end)
{
    int ret = from;
    for (int i = from + 1; i < end; ++i)
        if (scores[i] > scores[ret])
            ret = i;
    return ret;
}

MoveList::MoveList(Board *b, int s, MOVE fm, int *hs, MOVE k1, MOVE k2)
    : board(b)
    , side(s)
    , first_move(fm)
    , killer1(k1)
    , killer2(k2)
    , state(FIRST_MOVE)
    , history_scores(hs)
{
}

bool MoveList::remaining_moves()
{
    return state >= GENERATE_MOVES;
}

MOVE MoveList::next_move()
{
    MOVE ret = 0;
    switch (state)
    {
        case FIRST_MOVE:
            if (first_move != 0)
            {
                ret = first_move;
                state = GENERATE_CAPTURES;
                break;
            }

        case GENERATE_CAPTURES:
            board->generate_moves(side, moves, scores, &moves_count);
            c = 0;
            for (int i = c; i < moves_count; ++i)
                if (!(scores[i] > Board::NON_CAPTURE && is_winning_capture(board, moves[i], scores[i], side)))
                    scores[i] = Board::NON_CAPTURE;
            if (first_move != 0)
                remove_move(moves, scores, 0, moves_count, first_move);
            state = GOOD_CAPTURES;

        case GOOD_CAPTURES:
            {
                int besti = select_best(scores, c, moves_count);
                if (besti < moves_count && scores[besti] > Board::NON_CAPTURE)
                {
                    ret = moves[besti];
                    insert_move(moves, scores, besti, c);
                    ++c;
                    break;
                }
                else
                    state = GENERATE_MOVES;
            }

        case GENERATE_MOVES:
            if (first_move != 0)
                remove_move(moves, scores, c, moves_count, first_move);
            for (int i = c; i < moves_count; ++i)
            {
                if (moves[i] == killer1)
                    scores[i] = KILLER1_SCORE;
                else if (moves[i] == killer2)
                    scores[i] = KILLER2_SCORE;
                else
                    scores[i] = history_scores[moves[i]];
            }
            state = OTHERS;

        case OTHERS:
            {
                int besti = select_best(scores, c, moves_count);
                if (besti < moves_count)
                {
                    ret = moves[besti];
                    insert_move(moves, scores, besti, c);
                    ++c;
                    break;
                }
                else
                    break;
            }
    }
    return ret;
}
