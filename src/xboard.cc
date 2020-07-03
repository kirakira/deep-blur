#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include "board.h"
#include "search.h"

using std::cout;
using std::endl;
using std::string;
using blur::Board;
using blur::Move;
using blur::Side;
using blur::TranspositionTable;

const char kFeatureString[] =
    "feature myname=\"Deep Blur 2\" setboard=1 analyze=0 sigint=0 sigterm=0 "
    "reuse=0 variants=\"xiangqi\" nps=0 debug=1 done=1";

namespace {

bool IsPositionString(const string& str) {
  if (str.size() != 2) return false;
  return str[0] >= 'a' && str[0] <= 'i' && str[1] >= '0' && str[1] <= '9';
}

bool IsMoveString(const string& str) {
  if (str.size() != 4) return false;
  return IsPositionString(str.substr(0, 2)) && IsPositionString(str.substr(2));
}

void go(Board* board, TranspositionTable* tt, Side* side, int depth) {
  auto result = blur::Search(board, tt, *side, depth);
  blur::DebugPrintLogs();

  if (result.score != -blur::kMateScore) {
    CHECK(board->CheckedMake(*side, result.best_move).first);
    cout << "move " << result.best_move.ToString() << endl;
    *side = blur::OtherSide(*side);
  } else {
    if (*side == Side::kRed) {
      cout << "0-1 {White resigns}" << endl;
    } else {
      cout << "1-0 {Black resigns}" << endl;
    }
  }
}

}  // namespace

int main() {
  cout.setf(std::ios_base::unitbuf);

  Board board;
  TranspositionTable tt;
  bool force = false;
  Side side = Side::kRed;

  string line;
  while (std::getline(std::cin, line)) {
    std::istringstream iss(line);
    string command;
    iss >> command;

    if (command == "protover") {
      cout << kFeatureString << endl;
    } else if (command == "force") {
      force = true;
    } else if (command == "black") {
      side = Side::kBlack;
    } else if (command == "white") {
      side = Side::kRed;
    } else if (command == "quit") {
      break;
    } else if (command == "go") {
      force = false;
      int depth;
      if (!(iss >> depth)) depth = 30;
      go(&board, &tt, &side, depth);
    } else if (command == "xboard" || command == "new" || command == "random" ||
               command == "accepted" || command == "rejected" ||
               command == "variant" || command == "post" || command == "hard" ||
               command == "computer") {
      // No-op.
    } else if (command == "print") {
      board.DebugPrint();
      cout << board.ToString() << " " << (side == Side::kRed ? "w" : "b")
           << endl;
    } else if (command == "setboard") {
      string fen, turn;
      iss >> fen >> turn;
      if (board.SetBoard(fen)) {
        side = turn == "b" ? Side::kBlack : Side::kRed;
      } else {
        cout << "Error (invalid position): setboard" << endl;
      }
    } else if (command == "undo") {
      if (board.CheckedUnmake()) {
        side = blur::OtherSide(side);
      } else {
        cout << "Error (command not legal now): undo" << endl;
      }
    } else if (command == "remove") {
      if (!board.CheckedUnmake()) {
        cout << "Error (command not legal now): remove" << endl;
      } else if (!board.CheckedUnmake()) {
        side = blur::OtherSide(side);
        cout << "Error (command not legal now): remove. Treated as undo."
             << endl;
      }
    } else if (IsMoveString(command)) {
      Move move(command);
      auto move_result = board.CheckedMake(side, move);
      if (move_result.first) {
        if (move_result.second == blur::MoveType::kPerpetualAttacker) {
          cout << "# Warning: perpectual attack or check detected." << endl;
        }

        side = blur::OtherSide(side);
        if (!force) {
          go(&board, &tt, &side, 10);
        }
      } else {
        cout << "Illegal move: " << command << endl;
      }
    } else {
      cout << "Error (unknown command): " << line << endl;
    }
  }

  return 0;
}
