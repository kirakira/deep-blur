#include <iostream>
#include <string>
#include <vector>

#include "board.h"

using namespace blur;
using namespace std;

bool CheckBoardFen() {
  Board board;
  return board.ToString() ==
         "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR";
}

int main() {
  bool success = true;
  success = success && CheckBoardFen();
  cout << (success ? "Success." : "Failed.") << endl;
  return success ? 0 : 1;
}
