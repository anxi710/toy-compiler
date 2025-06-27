#pragma once

#include <cstddef>

namespace util {

struct Position {
  std::size_t row = 0;
  std::size_t col = 0;

  Position() = default;
  explicit Position(std::size_t r, std::size_t c) : row(r), col(c) {}
  Position(const Position &other) = default;

  auto operator=(const Position &rhs) -> Position& = default;
};

} // namespace util
