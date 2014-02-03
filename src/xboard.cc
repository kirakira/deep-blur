#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

#include "agent.h"

using namespace std;

string feature_string = "feature myname=\"Deep Blur\" setboard=1 analyze=0 sigint=0 sigterm=0 reuse=0 variants=\"xiangqi\" nps=0 debug=1 done=1";
ofstream fdebug("/tmp/output");

void debug_output(string s)
{
    fdebug << "" << s << endl;
}

bool is_square(char c)
{
    return (c >= 'a' && c <= 'i');
}

bool is_rank(char c)
{
    return (c >= '0' && c <= '9');
}

bool is_move(string s)
{
    return (s.length() == 4 && is_square(s[0])
            && is_rank(s[1]) && is_square(s[2]) && is_rank(s[3]));
}

void go(Board &board, Agent &agent, int &side, int depth = 6)
{
    MOVE res;

    int score = agent.search(board, side, &res, depth);
    if (score > -Agent::INF)
    {
        board.move(res);
        side = 1 - side;

        cout << "move " << move_string(res) << endl;
        debug_output("Sent a move: " + move_string(res));
    }
    else
    {
        if (side == 0)
            cout << "1-0 {White mates}" << endl;
        else
            cout << "0-1 {Black mates}" << endl;
    }
}

int main()
{
    Board board;
    Agent agent;

    string s;
    string line;
    int side = 1;
    bool force = false;

    while (getline(cin, line))
    {
        debug_output("Received: " + line);

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "protover")
            cout << feature_string << endl;
        else if (command == "force")
            force = true;
        else if (command == "black")
            side = 0;
        else if (command == "white")
            side = 1;
        else if (is_move(command))
        {
            if (!board.checked_move(make_move(command)))
                cout << "Illegal move: " << command << endl;
            else
            {
                side = 1 - side;

                if (!force)
                    go(board, agent, side);
            }
        }
        else if (command == "print")
            board.print();
        else if (command == "undo")
        {
            board.checked_unmove();
            side = 1 - side;
        }
        else if (command == "remove")
        {
            board.checked_unmove();
            board.checked_unmove();
        }
        else if (command == "setboard")
        {
            string fen, turn;
            iss >> fen >> turn;
            board.set(fen);
            if (turn == "b")
                side = 0;
            else
                side = 1;
        }
        else if (command == "generate")
        {
            int side = 0;
            iss >> side;

            MOVE moves[120];
            int capture_scores[120], moves_count;
            board.generate_moves(side, moves, capture_scores, &moves_count);

            for (int i = 0; i < moves_count; ++i)
                cout << move_string(moves[i]) << " ";
            cout << endl << moves_count << " moves in all." << endl;
        }
        else if (command == "quit")
            break;
        else if (command == "go")
        {
            force = false;
            int depth;
            if (iss >> depth)
                go(board, agent, side, depth);
            else
                go(board, agent, side);
        }
        else if (command == "qs")
        {
            cout << agent.quiescence(board, side, -Agent::INF, Agent::INF) << endl;
        }
        else if (command == "xboard" || command == "new" || command == "random" || command == "accepted" || command == "rejected"
                || command == "variant" || command == "post" || command == "hard" || command == "computer")
        {
            // do nothing
        }
        else if (command == "level")
        {
        }
        else if (command == "time")
        {
        }
        else if (command == "otim")
        {
        }
        else
            cout << "Error (unknown command): " << line << endl;
    }
    debug_output("quitting");

    return 0;
}
