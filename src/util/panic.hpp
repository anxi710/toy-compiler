#pragma once

#include <print>

#include "err_report.hpp"

namespace util {

[[noreturn]] inline void
unreachable(const std::string &msg) {
  std::println(stderr, "[unreachable]: {}", msg);
  exit(1);
}

[[noreturn]] inline void
runtime_error(const std::string &msg) {
  std::println(stderr, "[runtime error]: {}", msg);
  exit(1);
}

[[noreturn]] inline void
terminate() {
  exit(1);
}

} // namespace util
