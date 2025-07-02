#pragma once

#include <iostream>

namespace util {

[[noreturn]] inline void
unreachable(const char* msg) {
  std::cerr << "[unreachable] " << msg << std::endl;
  std::exit(1);
}

} // namespace util