#include "common.h"

#include <cstdlib>
#include <iostream>

namespace blur {

void Die(const std::string& message) {
  std::cerr << message << std::endl;
  std::exit(EXIT_FAILURE);
}

}  // namespace blur
