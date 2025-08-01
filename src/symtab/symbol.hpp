#pragma once

#include <memory>
#include <vector>
#include <variant>

#include "position.hpp"

namespace type {

struct Type;
using TypePtr = std::shared_ptr<Type>;

} // namespace type

namespace sym {

// 符号基类
struct Symbol {
  std::string    name;
  util::Position pos{0, 0}; // 符号声明时的位置

  virtual ~Symbol() = default;
  virtual auto str() -> std::string = 0;
};
using SymbolPtr = std::shared_ptr<Symbol>;

// NOTE: 所有非基本类型，包括数组和元组，都在栈上分配空间！

struct Value : Symbol {
  enum class Kind : std::uint8_t {
    TEMP,  // 临时变量
    LOCAL, // 局部变量
    CONST  // 常量
  } kind;

  bool mut;  // 本身是否可变

  // NOTE: rustc 将 数组/元组 已经初始化定义为
  //       在使用前至少用一次使用 数组元素/元组元素 进行的赋值！
  bool init; // 是否已经初始化

  int frameaddr; // 在栈上分配的相对地址

  type::TypePtr type; // 变量类型

  Value(Kind kind) : kind(kind) {}
  ~Value() override = default;

  [[nodiscard]] virtual bool isConst() const = 0;
  virtual auto str() -> std::string override = 0;
};
using ValuePtr = std::shared_ptr<Value>;

struct Temp : Value {
  Temp() : Value(Kind::TEMP) {}
  ~Temp() override = default;

  [[nodiscard]] bool isConst() const override {
    return false;
  }
  std::string str() final { return this->name; }
};
using TempPtr = std::shared_ptr<Temp>;

struct Variable : Value {
  bool formal; // 是否是形参
  std::string scopename;

  Variable() : Value(Kind::LOCAL) {}
  ~Variable() override = default;

  [[nodiscard]] bool isConst() const override {
    return false;
  }
  std::string str() final {
    return std::format(
      "{}::{}",
      this->scopename,
      this->name
    );
  }
};
using VariablePtr = std::shared_ptr<Variable>;

struct Constant : Value {
  std::variant<int, bool> val;

  Constant() : Value(Kind::CONST) {}
  ~Constant() override = default;

  [[nodiscard]] bool isConst() const override {
    return true;
  }
  std::string str() final { return this->name; }
};
using ConstantPtr = std::shared_ptr<Constant>;

struct Function : Symbol {
  std::vector<ValuePtr> argv;
  type::TypePtr         type; // return value type

  ~Function() override = default;
  std::string str() final { return this->name; }
};
using FunctionPtr = std::shared_ptr<Function>;

} // namespace sym
