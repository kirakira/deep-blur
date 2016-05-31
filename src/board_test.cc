#include <iostream>
#include <string>
#include <vector>

#include "board.h"

using namespace blur;
using namespace std;

bool CheckSetBoard() {
  Board board;
  if (board.ToString() !=
      "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR")
    return false;
  if (!board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != "4k4/9/9/9/9/9/9/9/9/4K4") return false;
  if (!board.SetBoard(
          "2eakae1r/5r3/hc4hc1/p1p1p1p1p/9/9/P1P1P1P1P/HC4HC1/3R4R/2EAKAE2"))
    return false;
  if (board.ToString() !=
      "2eakae1r/5r3/hc4hc1/p1p1p1p1p/9/9/P1P1P1P1P/HC4HC1/3R4R/2EAKAE2")
    return false;
  board.DebugPrint();
  return true;
}

bool CheckSetBadBoard() {
  Board board;
  const string board_fen =
      "rheakaehr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RHEAKAEHR";
  if (board.SetBoard("9/9/9/9/9/9/9/9/9/9")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4K4/9/9/9/9/9/9/9/9/4k4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k49/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4r/9/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4/")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4/9")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k3t/9/9/9/9/9/9/9/9/4K4")) return false;
  if (board.ToString() != board_fen) return false;
  if (board.SetBoard("4k4/9/9/9/9/9/9/9/9/4K4r")) return false;
  if (board.ToString() != board_fen) return false;
  return true;
}

int main() {
  bool success = true;
  success = success && CheckSetBoard();
  success = success && CheckSetBadBoard();
  cout << (success ? "Success." : "Failed.") << endl;
  return success ? 0 : 1;
}
