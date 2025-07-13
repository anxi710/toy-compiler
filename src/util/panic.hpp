#pragma once

#include <print>

#include "err_report.hpp"

namespace util {

/**
 * @brief 程序中不应该执行到的位置，若执行到则意味着程序运行出错
 *        应该打印错误信息并退出
 */
[[noreturn]] inline void
unreachable(const std::string &msg) {
  std::println(stderr, "[unreachable]: {}", msg);
  exit(1);
}

/**
 * @brief 程序运行时出现逻辑错误，打印错误信息并退出
 */
[[noreturn]] inline void
runtime_error(const std::string &msg) {
  std::println(stderr, "[runtime error]: {}", msg);
  exit(1);
}

/**
 * @brief 打印收集到的错误后终止程序
 */
[[noreturn]] inline void
terminate(err::ErrReporter &reporter) {
  if (reporter.hasErrs()) {
    reporter.displayErrs();
  }

  std::println(stderr, "");
  std::println(stderr, "程序出错，终止运行！");
  exit(1);
}

} // namespace util
