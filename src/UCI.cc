#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

string feature_string = "feature myname=\"Deep Blur\" analyze=0 variants=\"xiangqi\" nps=0 debug=1 done=1";
ofstream fdebug("/tmp/debug");

void debug_output(string s)
{
    fdebug << "" << s << endl;
}

int main()
{
    string s;
    ofstream fout("/tmp/output");

    string line;
    while (getline(cin, line))
    {
        fout << line << endl;

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "xboard")
            cout << feature_string << endl;
        else
            debug_output("Unknown command: " + line);
    }

    return 0;
}
