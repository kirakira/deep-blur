#include "bitboard.h"

#include <iostream>
#include <type_traits>
#include <vector>

using namespace std;

namespace blur {

uint64 TestFull(BitBoard board) {
  uint64 ans = 0;
  for (int i = 0; i < 50000000; ++i) {
    for (auto pos : board) {
      ans ^= pos.value();
    }
    /*
    for (auto pos : board.lower()) {
      ans ^= pos.value();
    }
    for (auto pos : board.upper()) {
      ans ^= pos.value();
    }*/
    ans <<= 1;
  }
  return ans;
}

uint64 TestHalf(HalfBitBoard board) {
  uint64 ans = 0;
  for (int i = 0; i < 50000000; ++i) {
    for (auto pos : board) {
      ans ^= pos.value();
    }
    ans <<= 1;
  }
  return ans;
}

}  // namespace blur

int main() {
  using namespace blur;

  int n;
  cin >> n;
  BitBoard board = BitBoard::EmptyBoard();
  //HalfBitBoard board = HalfBitBoard::EmptyBoard();
  for (int i = 0; i < n; ++i) {
    int x;
    cin >> x;
    board |= BitBoard::Fill(Position(x));
    //board |= HalfBitBoard::Fill(Position(x));
  }

  //cout << TestHalf(board) << endl;
  cout << TestFull(board) << endl;
  return 0;
}
