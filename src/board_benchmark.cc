#include "board.h"

#include <chrono>
#include <iostream>
#include <string>

const int kGenerateMovesCycles = 50000000;

using std::string;

class Timer {
 public:
  explicit Timer(const string& message)
      : message_(message), start_(std::chrono::system_clock::now()) {}

  std::chrono::duration<double> ElapsedSeconds() {
    return std::chrono::system_clock::now() - start_;
  }

  ~Timer() {
    const auto duration = ElapsedSeconds();
    std::cout << message_ << ": " << duration.count() << "s" << std::endl;
  }

 private:
  string message_;
  std::chrono::time_point<std::chrono::system_clock> start_;
};

namespace blur {

void RunGenerateMoves(Board board) {
  uint64 ans = 0;
  {
    Timer timer("GenerateMoves");
    for (int i = 0; i < kGenerateMovesCycles; ++i) {
      ans += board.GenerateMoves(Side::kRed).size();
    }
  }
  std::cout << "Total moves: " << ans << std::endl;
}

}  // namespace blur

int main() {
  blur::Board board;
  blur::RunGenerateMoves(board);
  return 0;
}
