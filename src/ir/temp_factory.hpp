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

namespace util {

struct Position;

} // namespace util

namespace ir {

class TempFactory {
public:
  ~TempFactory() = default;

public:
  void reset();
  sym::TempPtr produce(util::Position pos, type::TypePtr type);

private:
  int cnt = 0;
};

} // namespace ir
