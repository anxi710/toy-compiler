#pragma once

#include <memory>

#include "symbol.hpp"

namespace ir {

struct Operand {
  sym::ValuePtr value;

  Operand() = default;
  explicit Operand(sym::ValuePtr val) : value(std::move(val)) {}

  [[nodiscard]] std::string str() const {
    if (value != nullptr) {
      return value->name;
    }
    return "-";
  }
};

enum class IROp : std::uint8_t {
  ADD, SUB, MUL, DIV,
  EQ, NEQ, GEQ, GT, LEQ, LT,
  ASSIGN,
  GOTO, BNEZ, BLT,
  LABEL,
  FUNC, RETURN, PARAM, CALL,
  INDEX, // e.g., t2 = a[t1]
  DOT,   // e.g., t3 = a.1
  MAKE_ARR,
  MAKE_TUP,
};

[[nodiscard]] auto irop2str(const IROp &op) -> std::string;

struct IRQuad {
  IROp    op;
  Operand arg1;
  Operand arg2;
  Operand dst;

  std::vector<Operand> elems;
  std::string label; // 仅在跳转指令中有效！

  [[nodiscard]] std::string str() const;
};
using IRQuadPtr = std::shared_ptr<IRQuad>;

} // namespace ir
