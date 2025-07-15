#pragma once

#include <memory>

#include "symbol.hpp"

namespace ir {

struct Operand {
  enum class Kind : std::uint8_t {
    VALUE,
    ARRACC,
    TUPACC,
  } kind;

  virtual ~Operand() = default;

  [[nodiscard]] virtual auto str() const -> std::string = 0;
};
using OperandPtr = std::shared_ptr<Operand>;

struct Value : Operand {
  sym::ValuePtr value;

  ~Value() override = default;

  [[nodiscard]] std::string str() const override {
    return value->name;
  }
};
using ValuePtr = std::shared_ptr<Value>;

struct ArrAcc : Operand {
  sym::ValuePtr base;
  sym::ValuePtr idx;

  ~ArrAcc() override = default;

  [[nodiscard]] std::string str() const override {
    return std::format("{}[{}]", base->name, idx->name);
  }
};
using ArrAccPtr = std::shared_ptr<ArrAcc>;

struct TupAcc : Operand {
  sym::ValuePtr base;
  int idx;

  ~TupAcc() override = default;

  [[nodiscard]] std::string str() const override {
    return std::format("{}.{}", base->name, idx);
  }
};
using TupAccPtr = std::shared_ptr<TupAcc>;

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
  IROp       op;
  OperandPtr arg1;
  OperandPtr arg2;
  OperandPtr dst;

  std::string label; // 仅在跳转指令中有效！

  IRQuad() = default;
  ~IRQuad() = default;

  IRQuad(IROp op, OperandPtr arg1,
    OperandPtr arg2, OperandPtr dst
  ) : op(op), arg1(std::move(arg1)),
    arg2(std::move(arg2)), dst(std::move(dst)) {}

  IRQuad(IROp op, OperandPtr arg1,
    OperandPtr arg2, std::string label
  ) : op(op), arg1(std::move(arg1)),
    arg2(std::move(arg2)), label(std::move(label)) {}

  [[nodiscard]] std::string str() const;
};
using IRQuadPtr = std::shared_ptr<IRQuad>;

} // namespace ir
