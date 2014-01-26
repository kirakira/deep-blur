#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "agent.h"

using namespace std;

string feature_string = "feature myname=\"Deep Blur\" setboard=1 analyze=0 variants=\"xiangqi\" nps=0 debug=1 done=1";
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
    int side = 0;

    while (getline(cin, line))
    {
        debug_output("Received: " + line);

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "xboard")
            cout << feature_string << endl;
        else if (is_move(command))
        {
            if (!board.checked_move(Move(command)))
                cout << "Illegal move: " << command << endl;
            else
                side = 1 - side;
        }
        else if (command == "print")
            board.print();
        else if (command == "undo")
            board.checked_unmove();
        else if (command == "generate")
        {
            int side = 0;
            iss >> side;

            Move moves[120];
            int moves_count;
            board.generate_moves(side, moves, &moves_count);

            for (int i = 0; i < moves_count; ++i)
                cout << moves[i].to_string() << " ";
            cout << endl << moves_count << " moves in all." << endl;
        }
        else if (command == "quit")
            break;
        else if (command == "go")
        {
            Move res;
            if (agent.search(board, side, &res))
            {
                board.move(res);
                cout << res.to_string() << endl;
                side = 1 - side;
            }
        }
        else
            cout << "Error (unknown command): " << line << endl;
    }
    debug_output("quitting");

    return 0;
}
