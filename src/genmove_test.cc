#include "board.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

using std::string;
using std::vector;

namespace blur {

bool Verify(const string& fen, const string& side, const string& ans) {
  Board board;
  if (!board.SetBoard(fen)) {
    std::cerr << "Invalid board position: " << fen << std::endl;
    return false;
  }
  auto moves = board.GenerateMoves(side == "b" ? Side::kBlack : Side::kRed);
  vector<string> moves_string;
  std::transform(moves.begin(), moves.end(), std::back_inserter(moves_string),
                 [](Move m) { return m.ToString(); });
  std::sort(moves_string.begin(), moves_string.end());
  string out;
  for (const auto& s : moves_string) {
    if (!out.empty()) out += ",";
    out += s;
  }
  if (out != ans) {
    std::cerr << fen << " " << side << std::endl;
    std::cerr << "Output: " << out << " "
              << "(" << moves.size() << ")" << std::endl;
    std::cerr << "Answer: " << ans << std::endl;
  }
  return out == ans;
}

bool TestFile(const string& filename) {
  std::ifstream fin(filename);
  if (!fin.good()) {
    std::cerr << "Failed to open file: " << filename << "." << std::endl;
    return false;
  }
  string fen, side, ans;
  int i = 0;
  while (fin >> fen >> side >> ans) {
    ++i;
    if (i % 1000 == 0) std::cout << filename << " " << i << std::endl;
    if (!Verify(fen, side, ans)) {
      return false;
    }
  }
  return true;
}

}  // namespace blur

int main() {
  bool success = true;
  for (int i = 1; i <= 10; ++i) {
    success = success &&
              blur::TestFile("out/board_tests_" + std::to_string(i) + ".txt");
  }
  if (success) std::cout << "Success.";
  return success ? 0 : 1;
}
