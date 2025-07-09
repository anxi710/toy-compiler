#pragma once

#include <vector>

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
    delta = 0;
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

  void declareArg(const std::string &name, bool mut, type::TypePtr type) {
    auto arg = std::make_shared<sym::Variable>();
    arg->name   = name;
    arg->mut    = mut;
    arg->formal = true;
    arg->init   = true;
    arg->delta  = delta;
    arg->type   = std::move(type);

    delta += arg->type->memory;
    symtab.declareVar(name, arg);
    curfunc->argv.push_back(arg);
  }

  void declareVar(const std::string &name, bool mut, bool init, type::TypePtr type) {
    auto var = std::make_shared<sym::Variable>();
    var->name   = name;
    var->mut    = mut;
    var->formal = false;
    var->init   = init;
    var->delta  = delta;
    var->type   = std::move(type);

    delta += var->type->memory;
    symtab.declareVar(name, var);
  }

  void setRetValType(type::TypePtr type) {
    curfunc->type = std::move(type);
  }

  auto lookupVar(const std::string &name) const {
     return symtab.lookupVar(name);
  }

  auto lookupFunc(const std::string &name) const {
    return symtab.lookupFunc(name);
  }

public:
  sym::SymbolTable  &symtab;
  type::TypeFactory  types;

  // 当前函数上下文
  sym::FunctionPtr curfunc;
  int delta; // 局部变量堆栈偏移量
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
  std::vector<Scope> scopestack;

  // 特殊语句上下文

};

} // namespace sem

