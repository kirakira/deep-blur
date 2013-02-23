#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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
    string s;
    string line;

    int i = 0;
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
            debug_output("received move " + command);
            if (i == 0)// if (is_move(command))
            cout << "move b7e7" << endl;
        else if (i == 1)
            cout << "move a9a8" << endl;
/*        else
            debug_output("Unknown command: " + line);*/
        ++i;
        }
    }
    debug_output("quitting");

    return 0;
}
