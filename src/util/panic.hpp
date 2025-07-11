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
  //TODO: 完善程序终止的处理逻辑，例如错误打印
  exit(1);
}

} // namespace util
