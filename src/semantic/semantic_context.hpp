#pragma once

#include <ranges>
#include <vector>
#include <variant>

#include "position.hpp"
#include "type_factory.hpp"
#include "symbol_table.hpp"

namespace sem {

class SemanticContext {
public:
  SemanticContext(sym::SymbolTable &symtab) : symtab(symtab) {}

public:
  // utils
  void enterFunc(const std::string &name, util::Position pos) {
    curfunc = std::make_shared<sym::Function>();
    // 注意：形参列表和返回类型此时还未设置！
    curfunc->pos = pos;
    curfunc->name = name;

    symtab.declareFunc(name, curfunc);
    symtab.enterScope(name, true);
    scopenum = 0;
    scopestack.emplace_back(Scope::Kind::FUNC, name);
  }

  void enterIf() {
    const std::string &name = std::format("L{}", ++scopenum);
    symtab.enterScope(name, true);
    scopestack.emplace_back(Scope::Kind::IF, name);
  }

  void enterLoop() {
    const std::string &name = std::format("L{}", ++scopenum);
    symtab.enterScope(name, true);
    scopestack.emplace_back(Scope::Kind::LOOP, name);
  }

  void enterFor() {
    const std::string &name = std::format("L{}", ++scopenum);
    symtab.enterScope(name, true);
    scopestack.emplace_back(Scope::Kind::FOR, name);
  }

  void enterWhile() {
    const std::string &name = std::format("L{}", ++scopenum);
    symtab.enterScope(name, true);
    scopestack.emplace_back(Scope::Kind::WHILE, name);
  }

  void exitScope() {
    symtab.exitScope();
    scopestack.pop_back();
  }

  auto lookupFunc(const std::string &name) const {
    return symtab.lookupFunc(name);
  }

  auto lookupVal(const std::string &name) const {
     return symtab.lookupVal(name);
  }

  auto lookupConst(const std::string &name) const {
    return symtab.lookupConst(name);
  }

  void declareArg(const std::string &name, bool mut,
    type::TypePtr type, util::Position pos) {
    auto arg = std::make_shared<sym::Variable>();
    arg->pos    = pos;
    arg->name   = name;
    arg->mut    = mut;
    arg->formal = true;
    arg->init   = true;
    arg->type   = std::move(type);

    symtab.declareVal(name, arg);
    curfunc->argv.push_back(arg);
  }

  void declareVal(const std::string &name, bool mut, bool init,
    type::TypePtr type, util::Position pos) {
    auto val = std::make_shared<sym::Variable>();
    val->pos    = pos;
    val->name   = name;
    val->mut    = mut;
    val->init   = init;
    val->formal = false;
    val->type   = std::move(type);

    symtab.declareVal(name, val);
  }

  [[nodiscard]] sym::ConstantPtr declareConst(std::variant<int, bool> val, util::Position pos) {
    std::string name;
    type::TypePtr type;
    if (std::holds_alternative<int>(val)) {
      name = std::to_string(std::get<int>(val));
      type = type::TypeFactory::INT_TYPE;
    } else if (std::holds_alternative<bool>(val)) {
      if (std::get<bool>(val)) {
        name = "true";
      } else {
        name = "false";
      }
      type = type::TypeFactory::BOOL_TYPE;
    }

    if (auto opt_const = lookupConst(name); opt_const.has_value()) {
      return opt_const.value();
    }

    auto constant = std::make_shared<sym::Constant>();
    constant->pos = pos;
    constant->mut = false;
    constant->init = true;
    constant->name = name;
    constant->type = type;

    symtab.declareConst(name, constant);

    return constant;
  }

  void setRetValType(type::TypePtr type) {
    curfunc->type = std::move(type);
  }

  bool inLoopCtx() {
    for (const auto &scope : std::views::reverse(scopestack)) {
      if (scope.kind == Scope::Kind::LOOP ||
          scope.kind == Scope::Kind::WHILE ||
          scope.kind == Scope::Kind::FOR) {
        loopctx = scope;
        return true;
      }
    }
    return false;
  }

  auto checkAutoTypeInfer() const {
    return symtab.checkAutoTypeInfer();
  }

  auto getCurScopeName() const {
    std::string str = "global::";
    return symtab.getCurScopeName().substr(str.length());
  }

public:
  sym::SymbolTable  &symtab;
  type::TypeFactory  types;

  // 当前函数上下文
  sym::FunctionPtr curfunc;
  int scopenum; // 函数内部子作用域个数

  // 作用域上下文
  struct Scope {
    enum class Kind : std::uint8_t {
      FUNC,
      IF,
      LOOP,
      FOR,
      WHILE,
    } kind;
    std::string name;
  };
  Scope loopctx;
  std::vector<Scope> scopestack;
};

} // namespace sem
