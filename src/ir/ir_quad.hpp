#pragma once

#include "symbol_table.hpp"

namespace ir {

struct IRValue {
  std::optional<sym::VariablePtr> value;
};

enum class IROp : std::uint8_t {
  ADD, SUB, MUL, DIV,
  EQ, NEQ, GEQ, GT, LEQ, LT,
  ASSIGN,
  GOTO, BEQ, BNE, BGE, BGT, BLE, BLT,
  LABEL,
  FUNC, RETURN, PARAM, CALL,
  INDEX, // e.g., t2 = a[t1]
  DOT,   // e.g., t3 = a.1
};

[[nodiscard]] std::string irop2str(const IROp &op);

struct IRQuad {
  IROp op;
  IRValue dst;
  IRValue arg1;
  IRValue arg2;
  std::string label; // only for jump/label ops

  [[nodiscard]] std::string str() const;
};
using IRQuadPtr = std::shared_ptr<IRQuad>;

} // namespace ir

