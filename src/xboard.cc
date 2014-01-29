#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

#include "agent.h"

using namespace std;

string feature_string = "feature myname=\"Deep Blur\" setboard=1 analyze=0 sigint=0 sigterm=0 variants=\"xiangqi\" nps=0 debug=1 done=1";
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

int main()
{
    Board board;
    Agent agent;

    string s;
    string line;
    int side = 1;

    while (getline(cin, line))
    {
        debug_output("Received: " + line);

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "protover")
            cout << feature_string << endl;
        else if (is_move(command))
        {
            if (!board.checked_move(make_move(command)))
                cout << "Illegal move: " << command << endl;
            else
            {
                side = 1 - side;

                MOVE res;
                int score = agent.search(board, side, &res);
                if (score != -Agent::INF)
                {
                    board.move(res);
                    side = 1 - side;

                    cout << "move " << move_string(res) << endl;
                    debug_output("Sent a move " + move_string(res));
                }
            }
        }
        else if (command == "print")
            board.print();
        else if (command == "undo")
        {
            board.checked_unmove();
            side = 1 - side;
        }
        else if (command == "generate")
        {
            int side = 0;
            iss >> side;

            MOVE moves[120];
            int moves_count;
            board.generate_moves(side, moves, &moves_count);

            for (int i = 0; i < moves_count; ++i)
                cout << move_string(moves[i]) << " ";
            cout << endl << moves_count << " moves in all." << endl;
        }
        else if (command == "quit")
            break;
        else if (command == "go")
        {
            MOVE res;

            clock_t t = clock();
            int score = agent.search(board, side, &res);
            t = clock() - t;
            if (score != -Agent::INF)
            {
                board.move(res);
                side = 1 - side;

                cout << "move " << move_string(res) << endl;
                cout << "# " << score << "(" << ((double) t) / CLOCKS_PER_SEC << " s)" << endl;
            }
        }
        else if (command == "xboard" || command == "new" || command == "random" || command == "accepted" || command == "rejected" || command == "variant" || command == "post" || command == "hard")
        {
            // do nothing
        }
        else if (command == "level")
        {
        }
        else if (command == "time")
        {
        }
        else if (command == "otime")
        {
        }
        else
            cout << "Error (unknown command): " << line << endl;
    }
    debug_output("quitting");

    return 0;
}
