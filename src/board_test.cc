#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "board.h"

using SIDE = int;
using std::array;
using std::pair;
using std::string;
using std::vector;

std::mt19937 rng(0);

// Generates a uniformly random int in the range [a, b).
int RandomInt(int a, int b) {
  std::uniform_int_distribution<int> uid(a, b - 1);
  return uid(rng);
}

namespace strings {

template <typename ContainerType>
string Join(const ContainerType& pieces, const string& delimiter) {
  std::ostringstream result_stream;
  bool first_segment = true;
  for (const auto& piece : pieces) {
    if (!first_segment) result_stream << delimiter;
    first_segment = false;
    result_stream << piece;
  }
  return result_stream.str();
}

}  // namespace strings

void GeneratePositions(int count) {
  Board board;
  SIDE current_side = 1;
  array<int, 200> capture_scores;
  int moves_count;

  vector<pair<string, SIDE>> generated;
  for (; count; --count) {
    generated.emplace_back(board.fen_string(current_side), current_side);

    array<MOVE, 200> moves;
    board.generate_moves(current_side, moves.data(), capture_scores.data(),
                         &moves_count);

    vector<string> formatted_moves(moves_count);
    std::transform(moves.begin(), moves.begin() + moves_count,
                   formatted_moves.begin(),
                   [](MOVE move) { return move_string(move); });
    std::sort(formatted_moves.begin(), formatted_moves.end());

    std::cout << generated.back().first << " "
              << strings::Join(formatted_moves, ",") << std::endl;

    bool jump = generated.size() >= 10 && RandomInt(0, 10) < 2;
    if (!jump) {
      // Pick a random move.
      int move_index = RandomInt(0, moves_count);
      MoveType move_type;
      if (!board.checked_move(current_side, moves[move_index], &move_type)) {
        jump = true;
      } else {
        current_side = 1 - current_side;
      }
    }
    if (jump) {
      int jump_index = RandomInt(0, generated.size());
      board.set(generated[jump_index].first);
      current_side = generated[jump_index].second;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: board_test num_tests [seed]" << std::endl;
    return 1;
  }
  if (argc == 3) rng = std::mt19937(std::stoi(argv[2]));
  GeneratePositions(std::stoi(argv[1]));
  return 0;
}
