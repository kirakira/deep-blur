#include "board-base.h"

#include <cassert>

using std::string;

namespace blur {

Position::Position(const string& str) {
  assert(str.length() == 2);
  assert(str[0] >= 'a' && str[0] <= 'i');
  assert(str[1] >= '0' && str[1] <= '9');
  value_ = kNumColumns * (str[1] - '0') + (str[0] - 'a');
}

string Position::ToString() const {
  return string(1, "abcdefghi"[Column()]) + string(1, "0123456789"[Row()]);
}

Move::Move(const string& str) {
  assert(str.length() == 4);
  from_ = Position(str.substr(0, 2));
  to_ = Position(str.substr(2));
}

string Move::ToString() const {
  return from_.ToString() + to_.ToString();
}

}  // namespace blur
