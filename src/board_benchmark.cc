// Sample output:
//
//   GenerateMoves: 11.0235s
//   Total moves: 2200000000
//   GenerateCaptures: 6.91216s
//   Total captures: 100000000
//

#include "board.h"

#include <chrono>
#include <iostream>
#include <string>

const int kGenerateMovesCycles = 50000000;
const int kGenerateCapturesCycles = 50000000;

using std::string;

class LoggedTimer {
 public:
  explicit LoggedTimer(const string& message)
      : message_(message), start_(std::chrono::system_clock::now()) {}

  std::chrono::duration<double> ElapsedSeconds() {
    return std::chrono::system_clock::now() - start_;
  }

  ~LoggedTimer() {
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
    LoggedTimer timer("GenerateMoves");
    for (int i = 0; i < kGenerateMovesCycles; ++i) {
      ans += board.GenerateMoves(Side::kRed).size();
    }
  }
  std::cout << "Total moves: " << ans << std::endl;
}

void RunGenerateCaptures(Board board) {
  uint64 ans = 0;
  {
    LoggedTimer timer("GenerateCaptures");
    for (int i = 0; i < kGenerateCapturesCycles; ++i) {
      ans += board.GenerateCaptures(Side::kRed).size();
    }
  }
  std::cout << "Total captures: " << ans << std::endl;
}

}  // namespace blur

int main() {
  blur::Board board;
  blur::RunGenerateMoves(board);
  blur::RunGenerateCaptures(board);
  return 0;
}
