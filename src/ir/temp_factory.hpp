#pragma once

#include <memory>

namespace sym {

struct Temp;
using TempPtr = std::shared_ptr<Temp>;

} // namespace sym

namespace type {

struct Type;
using TypePtr = std::shared_ptr<Type>;

} // namespace type

namespace util { struct Position; }

namespace ir {

class TempFactory {
public:
  /**
   * @brief 重置临时变量计数器
   */
  void resetCnt() { cnt = 0; }

  auto produce(util::Position pos, type::TypePtr type) -> sym::TempPtr;

private:
  int cnt = 0; // temp counter
};

} // namespace ir
