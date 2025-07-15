#pragma once

#include <print>
#include <string_view>
#include <source_location>
#include <cstdlib>

namespace util {

// 带信息的断言：仅在 Debug 模式下启用
inline void
assertWithMsg(bool cond, std::string_view msg, const std::source_location &loc)
{
  if (!cond) {
    std::println("\n============== ASSERTION FAILED ==============\n");
    std::println("Location: {} at {}", loc.file_name(), loc.line());
    std::println("Function: {}", loc.function_name());
    std::println("Message : {}", msg);
    std::println("\n============== ASSERTION FAILED ==============\n");
    std::abort();
  }
}

inline void
expect(bool cond, std::string_view msg, const std::source_location &loc)
{
  if (!cond) {
    std::println("\n============== EXPECT FAILED ==============\n");
    std::println("Location: {} at {}", loc.file_name(), loc.line());
    std::println("Function: {}", loc.function_name());
    std::println("Message : {}", msg);
    std::println("\n============== EXPECT FAILED ==============\n");
    std::abort();
  }
}

// 致命错误：无条件终止
[[noreturn]] inline void
fatalError(std::string_view msg, const std::source_location& loc)
{
  std::println("\n============== FATAL ERROR ==============\n");
  std::println("Location: {} at {}", loc.file_name(), loc.line());
  std::println("Function: {}", loc.function_name());
  std::println("Message : {}", msg);
  std::println("\n============== FATAL ERROR ==============\n");
  std::abort();
}

} // namespace util

// Debug-only ASSERT（模拟标准 assert 行为）
#ifdef DEBUG
  #define ASSERT_MSG(cond, msg) \
    do { util::assertWithMsg((cond), (msg), std::source_location::current()); } while (0)

  #define EXPECT(cond, msg) \
    do { util::expect((cond), (msg), std::source_location::current()); } while (0)
#else
  #define ASSERT_MSG(cond, msg) \
    do { ((void)0); } while (0)

  #define EXPECT(cond, msg) \
    do { ((void)0); } while (0)
#endif

// Always-active CHECK（release 模式也会检查）
#define CHECK(cond, msg) \
  do { util::assertWithMsg((cond), (msg), std::source_location::current()); } while (0)

// Fatal, noreturn
#define UNREACHABLE(msg) \
  do { util::fatalError((msg), std::source_location::current()); } while (0)
