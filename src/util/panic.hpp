#pragma once

#include <print>
#include <string>
#include <cstdlib>
#include <source_location>

namespace util {

[[noreturn]] inline void
printErrMsgAbort(const std::string &kind, const std::string &msg, const std::source_location &loc)
{
  std::println("\n============== {} ==============\n", kind);
  std::println("Location: {} at {}", loc.file_name(), loc.line());
  std::println("Function: {}", loc.function_name());
  std::println("Message : {}", msg);
  std::println("\n============== {} ==============\n", kind);
  std::abort();
}

// 带信息的断言：仅在 Debug 模式下启用
inline void
assertWithMsg(bool cond, const std::string &msg, const std::source_location &loc)
{
  if (!cond) {
    printErrMsgAbort("ASSERTION FAILED", msg, loc);
  }
}

// 致命错误：无条件终止
[[noreturn]] inline void
fatalError(const std::string &msg, const std::source_location& loc)
{
  printErrMsgAbort("FATAL ERROR", msg, loc);
}

} // namespace util

// Debug-only ASSERT（模拟标准 assert 行为）
#ifdef DEBUG
  #define ASSERT_MSG(cond, msg) \
    do { util::assertWithMsg((cond), (msg), std::source_location::current()); } while (0)
#else
  #define ASSERT_MSG(cond, msg) \
    do { ((void)0); } while (0)
#endif

// Always-active CHECK（release 模式也会检查）
#define CHECK(cond, msg) \
  do { util::assertWithMsg((cond), (msg), std::source_location::current()); } while (0)

// Fatal, noreturn
#define UNREACHABLE(msg) \
  do { util::fatalError((msg), std::source_location::current()); } while (0)
