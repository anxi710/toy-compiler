#pragma once

#include <memory>

#include "symbol.hpp"

#define IROP_LIST(_) \
  _(ADD,      "+") \
  _(SUB,      "-") \
  _(MUL,      "*") \
  _(DIV,      "/") \
  _(EQ,       "==") \
  _(NEQ,      "!=") \
  _(GEQ,      ">=") \
  _(GT,       ">") \
  _(LEQ,      "<=") \
  _(LT,       "<") \
  _(ASSIGN,   "=") \
  _(GOTO,     "goto") \
  _(BEQZ,     "beqz") \
  _(BNEZ,     "bnez") \
  _(BGE,      "bge") \
  _(LABEL,    "label") \
  _(FUNC,     "func") \
  _(RETURN,   "return") \
  _(PARAM,    "param") \
  _(CALL,     "call") \
  _(INDEX,    "[]") \
  _(DOT,      ".") \
  _(MAKE_ARR, "make_array") \
  _(MAKE_TUP, "make_tuple")

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
#define DEFINE_ENUM(name, str) name,
  IROP_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
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
