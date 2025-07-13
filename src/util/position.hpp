#pragma once

#include <format>
#include <string>
#include <cstddef>

namespace util {

struct Position {
  std::size_t row = 0; // 行号
  std::size_t col = 0; // 列号

  Position() = default;
  Position(std::size_t r, std::size_t c) : row(r), col(c) {}
  Position(const Position &other) = default;

  Position& operator=(const Position &rhs) = default;
  Position operator+(const Position &rhs) const {
    return {row + rhs.row, col + rhs.col};
  }
};

} // namespace util

namespace std {

/**
 * @brief 格式化 struct Position 为 string，便于 format 打印
 */
template<>
struct formatter<util::Position> : formatter<std::string> {
  auto format(const util::Position &pos, format_context &ctx) const {
    return std::formatter<std::string>::format(
      std::format("(row: {}, col: {})", pos.row, pos.col), ctx
    );
  }
};

} // namespace std
